# VCalcBase
The base cmake setup for VCalc assignment.

Author: Braedy Kuzma (braedy@ualberta.ca)  
Updated by: Deric Cheung (dacheung@ualberta.ca)

# Usage
## Installing MLIR
In this assignment and your final project you will be working with MLIR and LLVM.
Due to the complex nature (and size) of the project we did not want to include
MLIR as a subproject. Therefore, there is some additional setup required to get your
build up and running.

### On a personal machine
The first thing you should do is have a look into the `configureLLVM.sh` script
in the scripts folder and understand as much as possible. We'll touch some
things briefly here that are better explained there. The steps here will expect
you have some knowledge of what's going on inside the script.

  1. Checkout LLVM to your machine
     1. `cd $HOME`
     1. `git clone https://github.com/llvm/llvm-project.git`
     1. `cd llvm-project`
     1. `git checkout llvmorg-16.0.4`
  1. Build MLIR
     1. `mkdir build`
     1. `cd build`
     1. ```cmake -G Ninja ../llvm \
             -DLLVM_ENABLE_PROJECTS=mlir \
             -DLLVM_BUILD_EXAMPLES=ON \
             -DLLVM_TARGETS_TO_BUILD="Native;NVPTX;AMDGPU" \
             -DCMAKE_BUILD_TYPE=Release \
             -DLLVM_ENABLE_ASSERTIONS=ON
        ```
     1. `cmake --build . --target check-mlir`
  1. Add these configuration lines to your ``~/.bashrc`` on linux or
     ``~/.zprofile`` on MacOS to setup the next steps. You should restart your
     terminal after editing these files.
      ```bash
      export MLIR_INS="$HOME/llvm-project/build/"
      export MLIR_DIR="$MLIR_INS/lib/cmake/mlir/" # Don't change me.
      export PATH="$MLIR_INS/bin:$PATH" # Don't change me
      ```
  1. CMake should automatically pick up your built mlir now. You should return
     to the assignment specification to complete setup.

### On university machines
You won't be building MLIR on the university machines: AICT wouldn't be very
happy with you. Instead, we are providing a **RELEASE** build available for
everyone.
  1. Follow the instructions on the [setup
     page](https://webdocs.cs.ualberta.ca/~c415/setup/) for the CS computers and
     MLIR/LLVM will be available to you.

## Building
### Linux
  1. Install git, java (only the runtime is necessary), and cmake (>= v3.0).
     - Until now, cmake has found the dependencies without issues. If you
       encounter an issue, let a TA know and we can fix it.
  1. Make a directory that you intend to build the project in and change into
     that directory.
  1. Run `cmake <path-to-VCalc-Base>`.
  1. Run `make`.
  1. Done.

## Pulling in upstream changes
If there are updates to your assignment you can retrieve them using the
instructions here.
  1. Add the upstream as a remote using `git remote add upstream <clone-link>`.
  1. Fetch updates from the upstream using `git fetch upstream`
  1. Merge the updates into a local branch using
     `git merge <local branch> upstream/<upstream branch>`. Usually both
     branches are `master`.
  1. Make sure that everything builds before committing to your personal
     master! It's much easier to try again if you can make a fresh clone
     without the merge!

Once the remote has been added, future updates are simply the `fetch` and
`merge` steps.
