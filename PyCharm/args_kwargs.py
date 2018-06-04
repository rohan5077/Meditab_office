student = {"pack" , "200", "name" , "Rohan"}

def main_info(*args, **kwargs):
    print(args)
    print(kwargs)

def argus(*args):
    print(args)

def kwargus(**kwargs):
    print(kwargs)

#main_info("name", 200, name = "Rohan")

argus("name", "subject")
#kwargus(name = "Rohan")

for key, value in student:
    print(F"{0}, {1}".format(*key,*value))