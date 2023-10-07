import matplotlib.pyplot as plt
import numpy as np

# Initialize empty lists to store x and y values
x_values = []
y_values = []

# Open the file and read the data
m = 0
b = 0
with open("data.txt", "r") as file:
    lines = file.readlines()
    for line in lines[:-1]:  # Skip the last line
        # Split each line into values using a comma as the delimiter
        values = line.strip().split(',')
        if len(values) == 2:
            x_values.append(float(values[0]))
            y_values.append(float(values[1]))
    last = lines[-1].strip().split(',')
    m = float(last[0])
    b = float(last[1])


# Create a line plot for the data
plt.scatter(x_values, y_values, marker='o', label='Data')

# Define the parameters for the linear equation y = mx + b

# Calculate the corresponding y values for the linear equation
linear_y_values = [m * x + b for x in x_values]

# Create a line plot for the linear equation
plt.plot(x_values, linear_y_values, linestyle='-', label=f'y = {m}x + {b}', color='red')

# Add labels, a title, and a legend
plt.xlabel('X Values')
plt.ylabel('Y Values')
plt.title('Plot of Data with Linear Equation')
plt.legend()

# Show the plot
plt.show()

