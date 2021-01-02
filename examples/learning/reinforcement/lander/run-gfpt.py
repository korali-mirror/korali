#!/usr/bin/env python3
import os
import sys
sys.path.append('./_model')
from env import *

####### Defining Korali Problem

import korali
k = korali.Engine()
e = korali.Experiment()

### Defining the Cartpole problem's configuration

e["Problem"]["Type"] = "Reinforcement Learning / Continuous"
e["Problem"]["Environment Function"] = env
e["Problem"]["Actions Between Policy Updates"] = 1
e["Problem"]["Training Reward Threshold"] = 0
e["Problem"]["Policy Testing Episodes"] = 10

### Defining State variables

e["Variables"][0]["Name"] = "Position X"
e["Variables"][1]["Name"] = "Position Y"
e["Variables"][2]["Name"] = "Velocity X"
e["Variables"][3]["Name"] = "Velocity Y"

### Defining Action variables 

e["Variables"][4]["Name"] = "Force X"
e["Variables"][4]["Type"] = "Action"
e["Variables"][4]["Lower Bound"] = -0.1
e["Variables"][4]["Upper Bound"] = +0.1
e["Variables"][4]["Exploration Sigma"]["Initial"] = 0.02
e["Variables"][4]["Exploration Sigma"]["Final"] = 0.005
e["Variables"][4]["Exploration Sigma"]["Annealing Rate"] = 1e-5

e["Variables"][5]["Name"] = "Force Y"
e["Variables"][5]["Type"] = "Action"
e["Variables"][5]["Lower Bound"] = +0.0
e["Variables"][5]["Upper Bound"] = +0.5
e["Variables"][5]["Exploration Sigma"]["Initial"] = 0.05
e["Variables"][5]["Exploration Sigma"]["Final"] = 0.005
e["Variables"][5]["Exploration Sigma"]["Annealing Rate"] = 1e-5

### Defining Agent Configuration 

e["Solver"]["Type"] = "Agent / Continuous / GFPT"
e["Solver"]["Mode"] = "Training"
e["Solver"]["Learning Rate"] = 0.001
e["Solver"]["Experiences Per Generation"] = 500
e["Solver"]["Experiences Between Policy Updates"] = 1
e["Solver"]["Cache Persistence"] = 100
e["Solver"]["Discount Factor"] = 0.99

### Defining the configuration of replay memory

e["Solver"]["Mini Batch Size"] = 32
e["Solver"]["Mini Batch Strategy"] = "Uniform"

### Defining the configuration of replay memory

e["Solver"]["Experience Replay"]["Start Size"] = 4096
e["Solver"]["Experience Replay"]["Maximum Size"] = 65536
e["Solver"]["Experience Replay"]["Serialization Frequency"] = 0

### Configuring the Remember-and-Forget Experience Replay algorithm

e["Solver"]["Experience Replay"]["REFER"]["Enabled"] = True
e["Solver"]["Experience Replay"]["REFER"]["Cutoff Scale"] = 4.0
e["Solver"]["Experience Replay"]["REFER"]["Target"] = 0.1
e["Solver"]["Experience Replay"]["REFER"]["Initial Beta"] = 0.6
e["Solver"]["Experience Replay"]["REFER"]["Annealing Rate"] = 5e-7

### Defining Critic and Policy Configuration

e["Solver"]["Critic"]["Advantage Function Population"] = 12
e["Solver"]["Policy"]["Learning Rate Scale"] = 0.1
e["Solver"]["Policy"]["Target Accuracy"] = 0.001
e["Solver"]["Policy"]["Optimization Candidates"] = 12

### Configuring the neural network and its hidden layers

e["Solver"]["Neural Network"]["Engine"] = "OneDNN"

e["Solver"]["Time Sequence Length"] = 1

e["Solver"]["Neural Network"]["Hidden Layers"][0]["Type"] = "Layer/Linear"
e["Solver"]["Neural Network"]["Hidden Layers"][0]["Output Channels"] = 32

e["Solver"]["Neural Network"]["Hidden Layers"][1]["Type"] = "Layer/Activation"
e["Solver"]["Neural Network"]["Hidden Layers"][1]["Function"] = "Elementwise/Tanh"

e["Solver"]["Neural Network"]["Hidden Layers"][2]["Type"] = "Layer/Linear"
e["Solver"]["Neural Network"]["Hidden Layers"][2]["Output Channels"] = 32

e["Solver"]["Neural Network"]["Hidden Layers"][3]["Type"] = "Layer/Activation"
e["Solver"]["Neural Network"]["Hidden Layers"][3]["Function"] = "Elementwise/Tanh"

### Defining Termination Criteria

e["Solver"]["Termination Criteria"]["Target Average Testing Reward"] = 0

### Setting file output configuration

e["Console Output"]["Verbosity"] = "Detailed"
e["File Output"]["Enabled"] = False
 
### Running Training Experiment

k.run(e)

#lander = Lander()
#lander.reset()
#done = False
#while not done:
 #state = lander.getState()
 #print(state)
 #done = lander.advance([0.0, 0.0])