message  = """Hello Python's _html"""

print (len(message))
print (message)
print (message[4])
print (message[0:5])    #slicing
print (message.upper())
print (message.count('o'))

greeting  = "Hello"
name = "Lpu"

msg  = '{}, {}. Welcome!'. format(greeting,name)
msg2  = f'{greeting}, {name.upper()}. Welcome!'
print(msg)
print(msg2)