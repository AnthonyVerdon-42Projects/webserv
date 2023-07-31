import cgitb
import cgi

cgitb.enable()

HTML = """\
<!DOCTYPE html>
<html>
    <head>
        <title>Hello there!</title>
    </head>
    <body>
        <h1>Hello there!</h1>
        {}
    </body>
</html>
"""

ERROR = """\
<h1>Error</h1>
<p>Please fill in the name and addr fields.</p>
"""

SUCCESS = """\
<p>name:{}</p>
<p>name:{}</p>
"""


def main() -> None:
    form = cgi.FieldStorage()

    print("Content-Type: text/html")
    print()
    if "name" not in form or "addr" not in form:
        print(HTML.format(ERROR))
    else:
        print(HTML.format(SUCCESS.format(form["name"].value, form["addr"].value)))


if __name__ == "__main__":
    main()
