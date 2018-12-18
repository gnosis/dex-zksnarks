from collections import defaultdict

import json


def convert_to_matrix(mat):
    """
    :param mat: list(dict{int: int})
    :return: list(list(int))
    """
    res = []
    size = max(max(k for k in row) for row in mat)
    for row in mat:
        r = [0 for _ in range(size + 1)]
        for k in row:
            r[k] = row[k]
        res.append(r)
    return res


def pepper_to_json(file_source_path, filename):
    res = {
        'primary_input': [],
        'aux_input': [],
        'constraints': []
    }

    with open(file_source_path + filename + 'primary') as file:
        content = file.readlines()
    for item in content:
        res['primary_input'].append(item.strip())

    with open(file_source_path + filename + 'aux') as file:
        content = file.readlines()
        for item in content:
            res['aux_input'].append(item.strip())

    a_rows = load_r1cs_matrix(file_source_path + filename + 'a')
    b_rows = load_r1cs_matrix(file_source_path + filename + 'b')
    c_rows = load_r1cs_matrix(file_source_path + filename + 'c')

    row_at = 0
    while any([a_rows, b_rows, c_rows]):
        res['constraints'].append([a_rows[row_at], b_rows[row_at], c_rows[row_at]])
        del a_rows[row_at]
        del b_rows[row_at]
        del c_rows[row_at]
        row_at += 1

    return json.dumps(res)


def export_pepper_to_json(file_source_path, filename):
    with open(file_source_path + filename + '.json', 'w') as file:
        file.write(pepper_to_json(file_source_path, filename))


def load_r1cs_matrix(file_name):
    res = defaultdict(dict)
    with open(file_name) as file:
        content = file.readlines()
    for line in content:
        column, row, value = map(int, line.strip().split())
        res[row][column] = value

    return res
