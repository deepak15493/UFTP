import sys
import os

text= "{name: \"John\", age: 395, city: \"New York\"}"

file = os.path.join(os.getcwd(), "Data", "Sender")

file = os.path.join(file, "temp.txt")

# with open(file, "wb") as f:
#     f.write(text)

with open(file, "rb") as f:
    buffer= f.read()

print(buffer)