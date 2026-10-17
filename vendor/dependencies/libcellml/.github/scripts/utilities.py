from packaging.version import parse


def _match_to_depth(version_1, version_2, depth):
    major_equal = version_1.major == version_2.major
    minor_equal = version_1.minor == version_2.minor
    patch_equal = version_1.micro == version_2.micro
    if depth == 1:
        return major_equal
    elif depth == 2:
        return major_equal and minor_equal

    return major_equal and minor_equal and patch_equal


def unwanted_versions(versions, depth=2):
    remove_versions = []
    index = 0
    while index < len(versions):
        next_ = index + 1
        v = parse(versions[index])
        v_next = None
        if next_ < len(versions):
            v_next = parse(versions[next_])

        if v_next is None:
            index += 1
        else:
            if _match_to_depth(v, v_next, depth):
                remove_versions.append(versions[next_])
                versions.pop(next_)
            else:
                index += 1

    return remove_versions

