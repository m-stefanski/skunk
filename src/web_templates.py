
def parse_template(filename, **variables):
    print("[html] parsing '{}' with values '{}'".format(filename, variables))
    with open(filename) as f:
        content = f.read()

    for key, value in variables.items():
        content = content.replace('#{}#'.format(key), str(value))

    return content
