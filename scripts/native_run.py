Import("env")

import subprocess


def run_native_program(target, source, env):
    program_path = str(env.subst("$PROGPATH"))
    return subprocess.call([program_path])

env.AddCustomTarget(
    name="run",
    dependencies=["$PROGPATH"],
    actions=[run_native_program],
    title="Run",
    description="Run native dashboard binary",
)
