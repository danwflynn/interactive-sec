import subprocess
import os

def run_command(command, cwd=None):
    """Run a shell command and handle errors."""
    result = subprocess.run(command, shell=True, cwd=cwd, text=True, capture_output=True)
    if result.returncode != 0:
        print(f"Command failed with error: {result.stderr}")
        raise subprocess.CalledProcessError(result.returncode, command)
    return result

def setup_glfw(build_dir):
    """Clone the GLFW repository, set up CMake, and build with MinGW."""
    repo_url = "https://github.com/glfw/glfw"
    
    # Clone the GLFW repository
    if not os.path.isdir("glfw"):
        print("Cloning GLFW repository...")
        run_command(f"git clone {repo_url}")
    
    # Create a build directory if it doesn't exist
    if not os.path.isdir(build_dir):
        os.makedirs(build_dir)
    
    # Run CMake to generate MinGW Makefiles
    print("Configuring CMake...")
    run_command(f"cmake -G \"MinGW Makefiles\" -S glfw -B {build_dir}")
    
    # Build the project using MinGW Makefiles
    print("Building GLFW...")
    run_command("mingw32-make", cwd=build_dir)

if __name__ == "__main__":
    build_dir = "glfw/build"
    setup_glfw(build_dir)
