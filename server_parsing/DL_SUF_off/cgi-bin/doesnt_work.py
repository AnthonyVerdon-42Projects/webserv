import cgitb

cgitb.enable()


def main() -> None:
    print("Content-Type: text/html")
    print()
    x = "salut";
    while x > 0:
        print(x)


if __name__ == "__main__":
    main()
