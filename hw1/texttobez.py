#!/usr/bin/env python3
"""
Phoenix Bezier Curve Generator

This script generates a series of Bezier curves that spell out "Phoenix"
and outputs the control points as x,y,z coordinates to a text file.
"""

def generate_letter_P(start_x, start_y, scale=1.0):
    """Generate Bezier curves for letter P"""
    curves = []
    x, y = start_x, start_y
    
    # Vertical line (left side)
    curves.append([
        [x, y, 0],                    # start
        [x, y - 30*scale, 0],         # control 1
        [x, y - 60*scale, 0],         # control 2
        [x, y - 90*scale, 0]          # end
    ])
    
    # Top horizontal curve
    curves.append([
        [x, y, 0],                    # start
        [x + 15*scale, y, 0],         # control 1
        [x + 30*scale, y, 0],         # control 2
        [x + 45*scale, y, 0]          # end
    ])
    
    # Top right curve
    curves.append([
        [x + 45*scale, y, 0],         # start
        [x + 55*scale, y - 10*scale, 0],  # control 1
        [x + 55*scale, y - 30*scale, 0],  # control 2
        [x + 45*scale, y - 45*scale, 0]   # end
    ])
    
    # Middle horizontal curve
    curves.append([
        [x + 45*scale, y - 45*scale, 0],  # start
        [x + 30*scale, y - 45*scale, 0],  # control 1
        [x + 15*scale, y - 45*scale, 0],  # control 2
        [x, y - 45*scale, 0]              # end
    ])
    
    return curves

def generate_letter_h(start_x, start_y, scale=1.0):
    """Generate Bezier curves for letter h"""
    curves = []
    x, y = start_x, start_y
    
    # Left vertical line
    curves.append([
        [x, y, 0],                    # start
        [x, y - 30*scale, 0],         # control 1
        [x, y - 60*scale, 0],         # control 2
        [x, y - 90*scale, 0]          # end
    ])
    
    # Top curve of h
    curves.append([
        [x, y - 30*scale, 0],         # start
        [x + 10*scale, y - 20*scale, 0],  # control 1
        [x + 25*scale, y - 20*scale, 0],  # control 2
        [x + 35*scale, y - 30*scale, 0]   # end
    ])
    
    # Right vertical line
    curves.append([
        [x + 35*scale, y - 30*scale, 0],  # start
        [x + 35*scale, y - 50*scale, 0],  # control 1
        [x + 35*scale, y - 70*scale, 0],  # control 2
        [x + 35*scale, y - 90*scale, 0]   # end
    ])
    
    return curves

def generate_letter_o(start_x, start_y, scale=1.0):
    """Generate Bezier curves for letter o (circular)"""
    curves = []
    x, y = start_x, start_y
    center_x = x + 20*scale
    center_y = y - 45*scale
    radius = 25*scale
    
    # Top half of circle (right to left)
    curves.append([
        [center_x + radius, center_y, 0],           # start (right)
        [center_x + radius, center_y + radius*0.55, 0],  # control 1
        [center_x - radius*0.55, center_y + radius, 0],  # control 2
        [center_x - radius, center_y, 0]           # end (left)
    ])
    
    # Bottom half of circle (left to right)
    curves.append([
        [center_x - radius, center_y, 0],          # start (left)
        [center_x - radius, center_y - radius*0.55, 0],  # control 1
        [center_x + radius*0.55, center_y - radius, 0],  # control 2
        [center_x + radius, center_y, 0]          # end (right)
    ])
    
    return curves

def generate_letter_e(start_x, start_y, scale=1.0):
    """Generate Bezier curves for letter e"""
    curves = []
    x, y = start_x, start_y
    
    # Main body curve (like c)
    curves.append([
        [x + 35*scale, y - 20*scale, 0],          # start
        [x + 35*scale, y - 10*scale, 0],          # control 1
        [x + 15*scale, y, 0],                     # control 2
        [x, y - 20*scale, 0]                      # end
    ])
    
    curves.append([
        [x, y - 20*scale, 0],                     # start
        [x, y - 45*scale, 0],                     # control 1
        [x, y - 70*scale, 0],                     # control 2
        [x + 15*scale, y - 90*scale, 0]           # end
    ])
    
    curves.append([
        [x + 15*scale, y - 90*scale, 0],          # start
        [x + 25*scale, y - 90*scale, 0],          # control 1
        [x + 35*scale, y - 80*scale, 0],          # control 2
        [x + 35*scale, y - 70*scale, 0]           # end
    ])
    
    # Middle bar
    curves.append([
        [x + 5*scale, y - 45*scale, 0],           # start
        [x + 15*scale, y - 45*scale, 0],          # control 1
        [x + 25*scale, y - 45*scale, 0],          # control 2
        [x + 30*scale, y - 45*scale, 0]           # end
    ])
    
    return curves

def generate_letter_n(start_x, start_y, scale=1.0):
    """Generate Bezier curves for letter n"""
    curves = []
    x, y = start_x, start_y
    
    # Left vertical line
    curves.append([
        [x, y - 45*scale, 0],         # start
        [x, y - 55*scale, 0],         # control 1
        [x, y - 75*scale, 0],         # control 2
        [x, y - 90*scale, 0]          # end
    ])
    
    # Top curve
    curves.append([
        [x, y - 45*scale, 0],         # start
        [x + 10*scale, y - 30*scale, 0],  # control 1
        [x + 25*scale, y - 30*scale, 0],  # control 2
        [x + 35*scale, y - 45*scale, 0]   # end
    ])
    
    # Right vertical line
    curves.append([
        [x + 35*scale, y - 45*scale, 0],  # start
        [x + 35*scale, y - 60*scale, 0],  # control 1
        [x + 35*scale, y - 75*scale, 0],  # control 2
        [x + 35*scale, y - 90*scale, 0]   # end
    ])
    
    return curves

def generate_letter_i(start_x, start_y, scale=1.0):
    """Generate Bezier curves for letter i"""
    curves = []
    x, y = start_x, start_y
    
    # Vertical line (stem)
    curves.append([
        [x + 5*scale, y - 35*scale, 0],   # start
        [x + 5*scale, y - 55*scale, 0],   # control 1
        [x + 5*scale, y - 75*scale, 0],   # control 2
        [x + 5*scale, y - 90*scale, 0]    # end
    ])
    
    # Dot (small circle)
    curves.append([
        [x + 8*scale, y - 15*scale, 0],   # start
        [x + 8*scale, y - 12*scale, 0],   # control 1
        [x + 5*scale, y - 10*scale, 0],   # control 2
        [x + 2*scale, y - 15*scale, 0]    # end
    ])
    
    curves.append([
        [x + 2*scale, y - 15*scale, 0],   # start
        [x + 2*scale, y - 18*scale, 0],   # control 1
        [x + 5*scale, y - 20*scale, 0],   # control 2
        [x + 8*scale, y - 15*scale, 0]    # end
    ])
    
    return curves

def generate_letter_x(start_x, start_y, scale=1.0):
    """Generate Bezier curves for letter x"""
    curves = []
    x, y = start_x, start_y
    
    # First diagonal (top-left to bottom-right)
    curves.append([
        [x, y - 45*scale, 0],             # start
        [x + 8*scale, y - 57*scale, 0],   # control 1
        [x + 22*scale, y - 73*scale, 0],  # control 2
        [x + 30*scale, y - 90*scale, 0]   # end
    ])
    
    # Second diagonal (top-right to bottom-left)
    curves.append([
        [x + 30*scale, y - 45*scale, 0],  # start
        [x + 22*scale, y - 57*scale, 0],  # control 1
        [x + 8*scale, y - 73*scale, 0],   # control 2
        [x, y - 90*scale, 0]              # end
    ])
    
    return curves

def generate_phoenix_bezier_curves():
    """Generate all Bezier curves for 'Phoenix'"""
    all_curves = []
    
    # Letter spacing
    letter_width = 1
    start_x = 0
    start_y = 0
    scale = 0.01
    
    # Generate each letter
    letters = [
        ('P', generate_letter_P),
        ('h', generate_letter_h),
        ('o', generate_letter_o),
        ('e', generate_letter_e),
        ('n', generate_letter_n),
        ('i', generate_letter_i),
        ('x', generate_letter_x)
    ]
    
    for i, (letter, generator_func) in enumerate(letters):
        x_offset = start_x + (i * letter_width)
        letter_curves = generator_func(x_offset, start_y, scale)
        all_curves.extend(letter_curves)
    
    return all_curves

def write_curves_to_file(curves, filename="phoenix_bezier_points.txt"):
    """Write all control points to a text file"""
    with open(filename, 'w') as f:
        for curve in curves:
            for point in curve:
                f.write(f"{point[0]},{point[1]},{point[2]}\n")
    
    total_points = sum(len(curve) for curve in curves)
    print(f"Generated {len(curves)} Bezier curves with {total_points} control points")
    print(f"Output written to: {filename}")

def main():
    """Main function to generate Phoenix Bezier curves"""
    print("Generating Bezier curves for 'Phoenix'...")
    
    # Generate the curves
    curves = generate_phoenix_bezier_curves()
    
    # Write to file
    write_curves_to_file(curves)
    
    print("Done! You can now use these points in your application.")

if __name__ == "__main__":
    main()