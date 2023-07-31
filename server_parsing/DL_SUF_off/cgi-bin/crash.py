import cgitb

cgitb.enable()


def main() -> None:
    print("Content-Type: text/html")
    print()
    array = [0, 0]
    print(array[2])


if __name__ == "__main__":
    main()
