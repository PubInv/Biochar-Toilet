import os
import sys
import subprocess
import platform
import shutil
from pathlib import Path

def get_cwebp_cmd(repo_root):
    system = platform.system()
    if system == "Windows":
        # Check for local executable first
        local_cwebp = repo_root / "libwebp-1.6.0-windows-x64" / "bin" / "cwebp.exe"
        if local_cwebp.exists():
            return str(local_cwebp)
        # Check if it's in the current directory
        local_cwd = Path.cwd() / "libwebp-1.6.0-windows-x64" / "bin" / "cwebp.exe"
        if local_cwd.exists():
            return str(local_cwd)
        # Fallback to checking PATH
        if shutil.which("cwebp"):
            return "cwebp"

        # If not found anywhere, return the path we want them to place it at
        # to trigger the FileNotFoundError with the correct missing path
        return str(Path.cwd() / "libwebp-1.6.0-windows-x64" / "bin" / "cwebp.exe")
    else:
        # Linux/macOS
        if shutil.which("cwebp"):
            return "cwebp"
        return "cwebp"

def convert_to_webp(input_folder):
    # Setup paths
    input_path = Path(input_folder).resolve()
    # Assuming script is in Presentations/ and repo root is one level up
    script_dir = Path(__file__).parent.resolve()
    repo_root = script_dir.parent
    output_path = repo_root / 'images'

    if not input_path.is_dir():
        print(f"Error: The input folder '{input_folder}' does not exist or is not a directory.")
        sys.exit(1)

    # Create output directory if it doesn't exist
    output_path.mkdir(parents=True, exist_ok=True)

    # Supported image extensions
    image_extensions = {'.jpg', '.jpeg', '.png', '.bmp', '.tiff'}

    cwebp_bin = get_cwebp_cmd(repo_root)

    # Process files
    converted_count = 0
    for file_path in input_path.iterdir():
        if file_path.is_file() and file_path.suffix.lower() in image_extensions:
            output_file = output_path / f"{file_path.stem}.webp"

            # cwebp command with quality 80
            cmd = [cwebp_bin, '-q', '80', str(file_path), '-o', str(output_file)]

            try:
                # Run the cwebp command
                subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
                print(f"Converted: {file_path.name} -> {output_file.name}")
                converted_count += 1
            except subprocess.CalledProcessError as e:
                print(f"Failed to convert {file_path.name}: {e.stderr.decode('utf-8')}")
            except FileNotFoundError:
                print("Error: 'cwebp' command not found. Please install WebP tools.")

                system = platform.system()
                if system == "Windows":
                    print("\nWindows Installation Instructions:")
                    print("1. Download the libwebp tools from:")
                    print("   https://storage.googleapis.com/downloads.webmproject.org/releases/webp/index.html")
                    print("2. Extract the archive.")
                    print("3. Copy the 'libwebp-1.6.0-windows-x64' folder to the root of this repository.")
                    print(f"   Expected path: {repo_root / 'libwebp-1.6.0-windows-x64' / 'bin' / 'cwebp.exe'}")
                elif system == "Darwin": # macOS
                    print("\nmacOS Installation Instructions:")
                    print("You can install WebP tools using Homebrew:")
                    print("   brew install webp")
                elif system == "Linux":
                    print("\nLinux Installation Instructions:")
                    print("You can install WebP tools using your package manager, for example:")
                    print("   sudo apt-get install webp")

                sys.exit(1)

    print(f"\nFinished. Converted {converted_count} images to WebP.")
    print(f"Output directory: {output_path}")

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python convert_to_webp.py <input_folder>")
        sys.exit(1)

    convert_to_webp(sys.argv[1])
