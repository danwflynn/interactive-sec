import os
import subprocess
import sys
import platform

# Define paths
project_name = "drawing_main"
source_files = ["./src/drawing_main.c", "./src/drawing_io.c"]
include_path = "./include"
glfw_include_path = "./glfw/include"
glfw_lib_path = "./glfw/build/src"

# Compiler
compiler = "gcc"

# Compiler flags
cflags = f"-I {include_path} -I {glfw_include_path}"

# Detect OS and set linker flags
if platform.system() == "Windows":
    lflags = f"-L {glfw_lib_path} -lglfw3 -lgdi32 -lopengl32 -lm -lpthread"
else:
    lflags = f"-L {glfw_lib_path} -lglfw3 -lGL -lX11 -lm -lpthread"

# Build command
command = f"{compiler} -o {project_name} {' '.join(source_files)} {cflags} {lflags}"

def build():
    """Build the project."""
    print(f"Compiling {' '.join(source_files)}...")
    
    try:
        # Run the build command
        result = subprocess.run(command, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print(result.stdout.decode("utf-8"))
        print(f"Build succeeded. Executable created: {project_name}")
    
    except subprocess.CalledProcessError as e:
        print("Build failed.")
        print(e.stderr.decode("utf-8"))
        sys.exit(1)

def clean():
    """Clean the build files."""
    executable = project_name + (".exe" if platform.system() == "Windows" else "")
    if os.path.exists(executable):
        os.remove(executable)
        print(f"Removed executable: {executable}")
    else:
        print(f"No executable found to remove: {executable}")

def main():
    """Main function to handle command-line arguments."""
    if len(sys.argv) != 2:
        print("Usage: python build.py [build|clean]")
        sys.exit(1)
    
    action = sys.argv[1].lower()

    if action == "build":
        build()
    elif action == "clean":
        clean()
    else:
        print(f"Unknown action: {action}")
        print("Usage: python build.py [build|clean]")
        sys.exit(1)

if __name__ == "__main__":
    main()
