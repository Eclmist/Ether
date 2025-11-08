#!/usr/bin/env python3
import os
import subprocess
import sys

def main():
    # Get folder path from argument or use current directory
    folder = sys.argv[1] if len(sys.argv) > 1 else "."
    
    if not os.path.isdir(folder):
        print(f"Error: {folder} is not a valid directory")
        sys.exit(1)
    
    # Read fps from fps.txt
    fps_file = os.path.join(folder, "fps.txt")
    if not os.path.exists(fps_file):
        print(f"Error: {fps_file} not found")
        sys.exit(1)
    
    try:
        with open(fps_file, "r") as f:
            fps = f.read().strip()
        fps = float(fps)
    except ValueError:
        print("Error: fps.txt does not contain a valid number")
        sys.exit(1)
    
    print(f"FPS: {fps}")
    
    # Find all PNG files and sort them numerically
    png_files = [f for f in os.listdir(folder) if f.endswith(".png")]
    
    if not png_files:
        print("Error: No PNG files found in the folder")
        sys.exit(1)
    
    # Sort by the numeric part of the filename
    png_files.sort(key=lambda x: int(x.split(".")[0]))
    
    print(f"Found {len(png_files)} PNG files")
    print(f"First frame: {png_files[0]}, Last frame: {png_files[-1]}")
    
    # Create ffmpeg input pattern
    input_pattern = os.path.join(folder, "%03d.png")
    output_file = "output.mp4"
    
    # Build ffmpeg command
    cmd = [
        "ffmpeg",
        "-framerate", str(fps),
        "-i", input_pattern,
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        output_file
    ]
    
    print(f"\nRunning: {' '.join(cmd)}\n")
    
    # Use the explicit path to ffmpeg
    ffmpeg_path = r"E:\Portable Programs\ffmpeg\ffmpeg.exe"
    cmd[0] = ffmpeg_path
    
    try:
        subprocess.run(cmd, check=True)
        print(f"\nVideo created successfully: {output_file}")
    except subprocess.CalledProcessError as e:
        print(f"Error running ffmpeg: {e}")
        sys.exit(1)
    except FileNotFoundError:
        print(f"Error: ffmpeg not found at {ffmpeg_path}")
        sys.exit(1)

if __name__ == "__main__":
    main()