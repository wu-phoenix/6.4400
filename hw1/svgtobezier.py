#!/usr/bin/env python3
"""
SVG to Bezier Control Points Converter

This script extracts Bezier curve control points from SVG files and outputs
them as x,y,z coordinates (with z=0) to a text file.

Requirements:
    pip install beautifulsoup4 lxml

Usage:
    python svg_to_bezier.py input.svg output.txt
    or modify the file paths in the script directly
"""

import re
from bs4 import BeautifulSoup
import sys

def parse_svg_path(path_data):
    """Extract Bezier control points from SVG path data"""
    points = []
    
    # Remove extra whitespace and normalize separators
    path_data = re.sub(r'[\s,]+', ' ', path_data.strip())
    
    # Split into commands and coordinates
    tokens = re.findall(r'[MmLlHhVvCcSsQqTtAaZz]|[-+]?(?:\d*\.\d+|\d+)', path_data)
    
    i = 0
    current_pos = [0, 0]
    
    while i < len(tokens):
        command = tokens[i]
        i += 1
        
        if command.upper() == 'M':  # Move to
            if i + 1 < len(tokens):
                x, y = float(tokens[i]), float(tokens[i+1])
                if command.islower() and current_pos != [0, 0]:
                    x += current_pos[0]
                    y += current_pos[1]
                current_pos = [x, y]
                points.append([x, y, 0])
                i += 2
                
        elif command.upper() == 'C':  # Cubic Bezier curve
            if i + 5 < len(tokens):
                coords = [float(tokens[j]) for j in range(i, i+6)]
                if command.islower():
                    # Relative coordinates
                    coords[0] += current_pos[0]
                    coords[1] += current_pos[1]
                    coords[2] += current_pos[0] 
                    coords[3] += current_pos[1]
                    coords[4] += current_pos[0]
                    coords[5] += current_pos[1]
                
                # Add control points: start, control1, control2, end
                points.append([current_pos[0], current_pos[1], 0])  # Start point
                points.append([coords[0], coords[1], 0])            # Control point 1
                points.append([coords[2], coords[3], 0])            # Control point 2
                points.append([coords[4], coords[5], 0])            # End point
                
                current_pos = [coords[4], coords[5]]
                i += 6
                
        elif command.upper() == 'S':  # Smooth cubic Bezier
            if i + 3 < len(tokens):
                coords = [float(tokens[j]) for j in range(i, i+4)]
                if command.islower():
                    coords[0] += current_pos[0]
                    coords[1] += current_pos[1]
                    coords[2] += current_pos[0]
                    coords[3] += current_pos[1]
                
                # For smooth curves, first control point is reflection of previous
                points.append([current_pos[0], current_pos[1], 0])  # Start point
                points.append([coords[0], coords[1], 0])            # Control point 2
                points.append([coords[2], coords[3], 0])            # End point
                
                current_pos = [coords[2], coords[3]]
                i += 4
                
        elif command.upper() == 'Q':  # Quadratic Bezier curve
            if i + 3 < len(tokens):
                coords = [float(tokens[j]) for j in range(i, i+4)]
                if command.islower():
                    coords[0] += current_pos[0]
                    coords[1] += current_pos[1]
                    coords[2] += current_pos[0]
                    coords[3] += current_pos[1]
                
                # Add control points: start, control, end
                points.append([current_pos[0], current_pos[1], 0])  # Start point
                points.append([coords[0], coords[1], 0])            # Control point
                points.append([coords[2], coords[3], 0])            # End point
                
                current_pos = [coords[2], coords[3]]
                i += 4
                
        elif command.upper() == 'L':  # Line to
            if i + 1 < len(tokens):
                x, y = float(tokens[i]), float(tokens[i+1])
                if command.islower():
                    x += current_pos[0]
                    y += current_pos[1]
                points.append([x, y, 0])
                current_pos = [x, y]
                i += 2
        else:
            # Skip other commands for now
            i += 1
    
    return points

def extract_bezier_points_from_svg(svg_file_path):
    """Extract all Bezier control points from an SVG file"""
    all_points = []
    
    try:
        with open(svg_file_path, 'r', encoding='utf-8') as file:
            svg_content = file.read()
    except Exception as e:
        print(f"Error reading SVG file: {e}")
        return all_points
    
    # Parse SVG with BeautifulSoup
    soup = BeautifulSoup(svg_content, 'xml')
    
    # Find all path elements
    paths = soup.find_all('path')
    
    for path in paths:
        d_attr = path.get('d')
        if d_attr:
            points = parse_svg_path(d_attr)
            all_points.extend(points)
    
    return all_points

def write_points_to_file(points, output_file_path):
    """Write control points to a text file"""
    try:
        with open(output_file_path, 'w') as file:
            for point in points:
                file.write(f"{point[0]},{point[1]},{point[2]}\n")
        print(f"Successfully wrote {len(points)} points to {output_file_path}")
    except Exception as e:
        print(f"Error writing to file: {e}")

def main():
    # You can modify these paths or use command line arguments
    if len(sys.argv) >= 3:
        svg_file = sys.argv[1]
        output_file = sys.argv[2]
    else:
        # Default file paths - modify these as needed
        svg_file = "input.svg"
        output_file = "bezier_points.txt"
    
    print(f"Processing SVG file: {svg_file}")
    
    # Extract Bezier control points
    points = extract_bezier_points_from_svg(svg_file)
    
    if points:
        # Write points to text file
        write_points_to_file(points, output_file)
    else:
        print("No Bezier curve points found in the SVG file.")

if __name__ == "__main__":
    main()
    