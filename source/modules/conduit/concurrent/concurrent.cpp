#include "engine.hpp"
#include "modules/conduit/concurrent/concurrent.hpp"
#include "modules/experiment/experiment.hpp"
#include "modules/problem/problem.hpp"
#include "modules/solver/solver.hpp"
#include "sample/sample.hpp"
#include <fcntl.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFERSIZE 4096

using namespace std;

namespace korali
{
namespace conduit
{
;

void Concurrent::initialize()
{
  // Setting workerId to -1 to identify master process
  _workerId = -1;

  if (_concurrentJobs < 1) KORALI_LOG_ERROR("You need to define at least 1 concurrent job(s) for external models \n");
  _resultSizePipe.clear();
  _resultContentPipe.clear();
  _inputsPipe.clear();
  while (!_workerQueue.empty()) _workerQueue.pop();

  for (size_t i = 0; i < _concurrentJobs; i++) _resultSizePipe.push_back(vector<int>(2));
  for (size_t i = 0; i < _concurrentJobs; i++) _resultContentPipe.push_back(vector<int>(2));
  for (size_t i = 0; i < _concurrentJobs; i++) _inputsPipe.push_back(vector<int>(2));
  for (size_t i = 0; i < _concurrentJobs; i++) _workerQueue.push(i);

  // Opening Inter-process communicator pipes
  for (size_t i = 0; i < _concurrentJobs; i++)
  {
    if (pipe(_inputsPipe[i].data()) == -1) KORALI_LOG_ERROR("Unable to create inter-process pipe. \n");
    if (pipe(_resultSizePipe[i].data()) == -1) KORALI_LOG_ERROR("Unable to create inter-process pipe. \n");
    if (pipe(_resultContentPipe[i].data()) == -1) KORALI_LOG_ERROR("Unable to create inter-process pipe. \n");
    fcntl(_resultSizePipe[i][0], F_SETFL, fcntl(_resultSizePipe[i][0], F_GETFL) | O_NONBLOCK);
    fcntl(_resultSizePipe[i][1], F_SETFL, fcntl(_resultSizePipe[i][1], F_GETFL) | O_NONBLOCK);
    fcntl(_resultContentPipe[i][0], F_SETFL, fcntl(_resultContentPipe[i][0], F_GETFL));
    fcntl(_resultContentPipe[i][1], F_SETFL, fcntl(_resultContentPipe[i][1], F_GETFL));
  }
}

void Concurrent::terminateServer()
{
  auto terminationJs = knlohmann::json();
  terminationJs["Conduit Action"] = "Terminate";

  string terminationString = terminationJs.dump();
  size_t terminationStringSize = terminationString.size();

  for (size_t i = 0; i < _concurrentJobs; i++)
  {
    write(_inputsPipe[i][1], &terminationStringSize, sizeof(size_t));
    write(_inputsPipe[i][1], terminationString.c_str(), terminationStringSize * sizeof(char));
  }

  for (size_t i = 0; i < _concurrentJobs; i++)
  {
    int status;
    ::wait(&status);
  }

  for (size_t i = 0; i < _concurrentJobs; i++)
  {
    close(_resultContentPipe[i][1]); // Closing pipes
    close(_resultContentPipe[i][0]); // Closing pipes
    close(_resultSizePipe[i][1]);    // Closing pipes
    close(_resultSizePipe[i][0]);    // Closing pipes
    close(_inputsPipe[i][1]);        // Closing pipes
    close(_inputsPipe[i][0]);        // Closing pipes
  }
}

void Concurrent::initServer()
{
  for (size_t i = 0; i < _concurrentJobs; i++)
  {
    pid_t processId = fork();
    if (processId == 0)
    {
      _workerId = i;
      worker();
      exit(0);
    }
    _workerPids.push_back(processId);
  }
}

void Concurrent::broadcastMessageToWorkers(knlohmann::json &message)
{
  string messageString = message.dump();
  size_t messageStringSize = messageString.size();

  for (size_t i = 0; i < _concurrentJobs; i++)
  {
    write(_inputsPipe[i][1], &messageStringSize, sizeof(size_t));

    size_t curPos = 0;
    while (curPos < messageStringSize)
    {
      size_t bufSize = BUFFERSIZE;
      if (curPos + bufSize > messageStringSize) bufSize = messageStringSize - curPos;
      write(_inputsPipe[i][1], messageString.c_str() + curPos, bufSize * sizeof(char));
      curPos += bufSize;
    }
  }
}

void Concurrent::sendMessageToEngine(knlohmann::json &message)
{
  string messageString = message.dump();
  size_t messageSize = messageString.size();

  write(_resultSizePipe[_workerId][1], &messageSize, sizeof(size_t));

  size_t curPos = 0;
  while (curPos < messageSize)
  {
    size_t bufSize = BUFFERSIZE;
    if (curPos + bufSize > messageSize) bufSize = messageSize - curPos;
    write(_resultContentPipe[_workerId][1], messageString.c_str() + curPos, bufSize * sizeof(char));
    curPos += bufSize;
  }
}

knlohmann::json Concurrent::recvMessageFromEngine()
{
  size_t inputStringSize;
  read(_inputsPipe[_workerId][0], &inputStringSize, sizeof(size_t));

  std::vector<char> inputString(inputStringSize + BUFFERSIZE);

  size_t curPos = 0;
  while (curPos < inputStringSize)
  {
    size_t bufSize = BUFFERSIZE;
    if (curPos + bufSize > inputStringSize) bufSize = inputStringSize - curPos;
    read(_inputsPipe[_workerId][0], &inputString[curPos], bufSize * sizeof(char));
    curPos += bufSize;
    sched_yield(); // Guarantees MacOs finishes the pipe reading
  }
  inputString[inputStringSize] = '\0';

  auto message = knlohmann::json::parse(inputString);

  return message;
}

void Concurrent::listenWorkers()
{
  // Check for child defunction
  for (size_t i = 0; i < _workerPids.size(); i++)
  {
    int status;
    pid_t result = waitpid(_workerPids[i], &status, WNOHANG);
    if (result != 0) KORALI_LOG_ERROR("Worker %i (Pid: %d) exited unexpectedly.\n", i, _workerPids[i]);
  }

  // Reading pending messages from all workers
  for (size_t i = 0; i < _workerPids.size(); i++)
  {
    // Identifying current sample
    auto sample = _workerToSampleMap[i];

    size_t resultStringSize;
    int readBytes = read(_resultSizePipe[i][0], &resultStringSize, sizeof(size_t));

    if (readBytes > 0)
    {
      char resultString[resultStringSize + BUFFERSIZE];

      size_t curPos = 0;
      while (curPos < resultStringSize)
      {
        size_t bufSize = BUFFERSIZE;
        if (curPos + bufSize > resultStringSize) bufSize = resultStringSize - curPos;
        read(_resultContentPipe[i][0], resultString + curPos, bufSize * sizeof(char));
        curPos += bufSize;
      }
      resultString[resultStringSize] = '\0';

      auto message = knlohmann::json::parse(resultString);
      sample->_messageQueue.push(message);
    }
  }
}

void Concurrent::stackEngine(Engine *engine)
{
  // (Engine-Side) Adding engine to the stack to support Korali-in-Korali execution
  _engineStack.push(engine);

  knlohmann::json engineJs;
  engineJs["Conduit Action"] = "Stack Engine";
  engine->serialize(engineJs["Engine"]);

  broadcastMessageToWorkers(engineJs);
}

void Concurrent::popEngine()
{
  // (Engine-Side) Removing the current engine to the conduit's engine stack
  _engineStack.pop();

  auto popJs = knlohmann::json();
  popJs["Conduit Action"] = "Pop Engine";
  broadcastMessageToWorkers(popJs);
}

void Concurrent::sendMessageToSample(Sample &sample, knlohmann::json &message)
{
  string messageString = message.dump();
  size_t messageStringSize = messageString.size();

  write(_inputsPipe[sample._workerId][1], &messageStringSize, sizeof(size_t));
  write(_inputsPipe[sample._workerId][1], messageString.c_str(), messageStringSize * sizeof(char));
}

bool Concurrent::isRoot()
{
  return _workerId == -1;
}

size_t Concurrent::getProcessId()
{
  return _workerId;
}

void Concurrent::setConfiguration(knlohmann::json& js) 
{
 if (isDefined(js, "Results"))  eraseValue(js, "Results");

 if (isDefined(js, "Concurrent Jobs"))
 {
 try { _concurrentJobs = js["Concurrent Jobs"].get<size_t>();
} catch (const std::exception& e)
 { KORALI_LOG_ERROR(" + Object: [ concurrent ] \n + Key:    ['Concurrent Jobs']\n%s", e.what()); } 
   eraseValue(js, "Concurrent Jobs");
 }
  else   KORALI_LOG_ERROR(" + No value provided for mandatory setting: ['Concurrent Jobs'] required by concurrent.\n"); 

 Conduit::setConfiguration(js);
 _type = "concurrent";
 if(isDefined(js, "Type")) eraseValue(js, "Type");
 if(isEmpty(js) == false) KORALI_LOG_ERROR(" + Unrecognized settings for Korali module: concurrent: \n%s\n", js.dump(2).c_str());
} 

void Concurrent::getConfiguration(knlohmann::json& js) 
{

 js["Type"] = _type;
   js["Concurrent Jobs"] = _concurrentJobs;
 Conduit::getConfiguration(js);
} 

void Concurrent::applyModuleDefaults(knlohmann::json& js) 
{

 std::string defaultString = "{\"Concurrent Jobs\": 1}";
 knlohmann::json defaultJs = knlohmann::json::parse(defaultString);
 mergeJson(js, defaultJs); 
 Conduit::applyModuleDefaults(js);
} 

void Concurrent::applyVariableDefaults() 
{

 Conduit::applyVariableDefaults();
} 

;

} //conduit
} //korali
;
