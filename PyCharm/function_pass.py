def hello_func():
    return 'delhi'

print(hello_func())

def hello_func1():
    print('Hello Function')

hello_func1()
print(len(hello_func()))

def format_func(greeting):
    return '{} Function.'.format(greeting)

def format_func1(greeting):
    return f'{greeting} Function.'

print(format_func('hi'))
print(format_func1('hi'))

def func2(greeting,name='you'):
    return '{},{}'.format(greeting,name)
print(func2('hi'))