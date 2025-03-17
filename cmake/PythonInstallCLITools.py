from pathlib import Path
from subprocess import run
from sys import argv, executable

# expecting one command line argument - the path to the python_lib folder to place the pip package
pkgs = {'energyplus_launch': '3.7.2', 'energyplus_transition_tools': '2.1.3'}
if to_install := [f"{n}=={v}" for n, v in pkgs.items() if not (Path(argv[1]) / f"{n}-{v}.dist-info").exists()]:
    print(f"PYTHON: CLI packages not found or out of date, pip installing these now: {to_install}")
    run([executable, '-m', 'pip', 'install', f'--target={argv[1]}', '--upgrade', *to_install], check=True)
