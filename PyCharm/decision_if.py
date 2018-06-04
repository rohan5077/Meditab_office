language = 'python'

if language == 'python':
    print('python')
elif language=='java':
    print('Java')
else:
    print('No Match')

logged_in = False

if language == 'python' or logged_in:
    print('admin')
else:
    print('wrong creds')