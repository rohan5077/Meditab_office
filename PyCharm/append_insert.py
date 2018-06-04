lists = ['Maths','Physics','Chemistry']
#lists.insert(0,'Arts')
print(lists)

list2 = ['Arts','History']
list2.extend(lists)
print(list2)

list2.remove('Maths')
print(list2)