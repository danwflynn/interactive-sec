import os
import subprocess
import sys

# Define paths
project_name = "drawing_main"
source_file = "./src/drawing_main.c"
glfw_include_path = "./glfw/include"
glfw_lib_path = "./glfw/build/src"

# Compiler and flags
compiler = "gcc"
cflags = f"-I {glfw_include_path}"
lflags = f"-L {glfw_lib_path} -lglfw3 -lgdi32 -lopengl32 -lm -lpthread"

# Build command
command = f"{compiler} -o {project_name} {source_file} {cflags} {lflags}"

def build():
    """Build the project."""
    print(f"Compiling {source_file}...")
    
    try:
        # Run the build command
        result = subprocess.run(command, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print(result.stdout.decode("utf-8"))
        print(f"Build succeeded. Executable created: {project_name}.exe")
    
    except subprocess.CalledProcessError as e:
        print("Build failed.")
        print(e.stderr.decode("utf-8"))
        sys.exit(1)

def clean():
    """Clean the build files."""
    if os.path.exists(project_name+".exe"):
        os.remove(project_name+".exe")
        print(f"Removed executable: {project_name}.exe")
    else:
        print(f"No executable found to remove: {project_name}.exe")

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
