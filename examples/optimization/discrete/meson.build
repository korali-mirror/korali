e = find_program('./.test-run.py', required: true)
test('optimization.discrete', e,
      timeout : 1000,
      suite: 'regression',
      workdir: meson.current_source_dir(),
      depends: python_extension,
      env: nomalloc
    )
