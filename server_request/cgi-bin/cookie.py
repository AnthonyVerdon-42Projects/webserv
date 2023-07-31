#!/usr/bin/env python3

import cgi
import cgitb
import http.cookies

cgitb.enable()

# Get the CGI data
form = cgi.FieldStorage()

# Create a new cookie
cookie = http.cookies.SimpleCookie()

# Check if the 'name' parameter is provided
if 'name' in form:
    name = form['name'].value

    # Set the cookie value
    cookie['name'] = name

    # Set cookie expiration time (in seconds)
    cookie['name']['expires'] = 3600  # Cookie expires in 1 hour

# Set the Content-Type header to 'text/html'
print("Content-Type: text/html")
print(cookie)
print()

# Print the HTML content
print("<html>")
print("<head>")
print("<title>Cookie Example</title>")
print("</head>")
print("<body>")
print("<h1>Cookie Example</h1>")

# Print the cookie value if it exists
if 'name' in cookie:
    print("<p>Welcome back, {}!</p>".format(cookie['name'].value))
else:
    print("<p>Please enter your name:</p>")
    print('<form method="post" action="">')
    print('<input type="text" name="name" required>')
    print('<input type="submit" value="Submit">')
    print('</form>')

print("</body>")
print("</html>")
print()
print()
