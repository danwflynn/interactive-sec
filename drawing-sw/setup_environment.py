import subprocess
import os
import platform

def run_command(command, cwd=None):
    """Run a shell command and handle errors."""
    result = subprocess.run(command, shell=True, cwd=cwd, text=True, capture_output=True)
    if result.returncode != 0:
        print(f"Command failed with error: {result.stderr}")
        raise subprocess.CalledProcessError(result.returncode, command)
    return result

def setup_glfw(build_dir):
    """Clone the GLFW repository, set up CMake, and build."""
    repo_url = "https://github.com/glfw/glfw"
    
    # Detect the operating system
    is_windows = platform.system() == "Windows"
    
    # Clone the GLFW repository if it doesn't already exist
    if not os.path.isdir("glfw"):
        print("Cloning GLFW repository...")
        run_command(f"git clone {repo_url}")
    
    # Create a build directory if it doesn't exist
    if not os.path.isdir(build_dir):
        os.makedirs(build_dir)
    
    # Determine generator and command based on OS
    generator_option = " -G \"MinGW Makefiles\"" if is_windows else ""
    build_command = "mingw32-make" if is_windows else "make"
    
    # Run CMake to configure the project (omit -G option for Raspberry Pi)
    print("Configuring CMake for GLFW...")
    run_command(f"cmake{generator_option} -S glfw -B {build_dir}".strip())
    
    # Build the project
    print("Building GLFW...")
    run_command(build_command, cwd=build_dir)

if __name__ == "__main__":
    glfw_build_dir = "glfw/build"
    
    # Set up GLFW
    setup_glfw(glfw_build_dir)
