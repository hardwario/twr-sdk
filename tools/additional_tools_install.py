Import("env")

# Install custom packages from the PyPi registry
env.Execute("$PYTHONEXE -m pip install --upgrade bcf")
