import os

TESTS_RESOURCE_LOCATION = "${TESTS_RESOURCE_LOCATION}"


def resource_path(relative_path=''):
    return TESTS_RESOURCE_LOCATION + '/' + relative_path


def file_contents(file_name):
    with open(os.path.join(TESTS_RESOURCE_LOCATION, file_name)) as f:
        content = f.read()

    return content
