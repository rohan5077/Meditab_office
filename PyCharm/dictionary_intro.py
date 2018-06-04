student = {'name':'Rohan', 'age' : 20, 'college': ['lpu','meditab']}

print(student)
print(student['college'])
print(student.get("age"))

print(student.get("phone",'Not found'))

student.update({'name':'Sharma', 'phone':'200'})
print(student)

age = student.pop('age')
print(student)
print(age)
print(len(student))

print(student.keys())
print(student.values())
print(student.items())