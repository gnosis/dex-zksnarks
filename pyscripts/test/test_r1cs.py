import unittest

import json
from pyscripts.r1cs.utils import pepper_to_json, export_pepper_to_json


class TestR1CS(unittest.TestCase):

    def setUp(self):
        self.path = 'data/'
        self.expected_satisfiable = {
            "primary_input": list(map(str, [
                1,
                1532495540865888858358347027150309178647734624407898969,
                1532495540865888858358347027150309178647734624407898969,
                1532495540865888858358347027150309173676703738353129137,
                49422296140220940097978124076994616448
            ])),
            "aux_input": list(map(str, [
                49422296140220940088036062304885076784,
                923282240202277392464324191650775209929277614826518468,
                923282240202277441886620331871715297965339919711595252,
                121640394664342172897679635793680824971542415576779149,
                1044922634866619614784299967665396122936882335288374401,
                870386587180443725789318892096925837942117467850909575,
                382813681181174482215271832612012777260234292676615175,
                803677629557484593758755079362619998949775449252019250
            ])),
            "constraints": [
                [{"1": 1, "2": 1}, {"0": 1}, {"3": 1}],
                [{"2": 1}, {"3": 1}, {"4": 1}],
                [{"3": 1, "4": 1}, {"0": 1}, {"5": 1}],
                [{"4": 1}, {"5": 1}, {"6": 1}],
                [{"5": 1, "6": 1}, {"0": 1}, {"7": 1}],
                [{"6": 1}, {"7": 1}, {"8": 1}],
                [{"7": 1, "8": 1}, {"0": 1}, {"9": 1}],
                [{"8": 1}, {"9": 1}, {"10": 1}],
                [{"9": 1, "10": 1}, {"0": 1}, {"11": 1}],
                [
                    {"1": 1, "2": 1, "3": 1, "4": 1, "5": 1, "6": 1, "7": 1, "8": 1, "9": 1, "10": 1, "11": 1},
                    {"1": 1, "2": 1, "3": 1, "4": 1, "5": 1, "6": 1, "7": 1, "8": 1, "9": 1, "10": 1, "11": 1},
                    {"12": 1}
                ]
            ]
        }

    def test_small_pepper_to_JSON(self):

        result = pepper_to_json(self.path + 'small/', 'r1cs.small_')

        expected = json.dumps({
            'primary_input': ["1"],
            'aux_input': [],
            'constraints': [
                [{"0": 1}, {"0": 1}, {"0": 1}]
            ]
        })

        self.assertEqual(expected, result)

    def test_medium_pepper_to_JSON(self):

        result = pepper_to_json(self.path, 'medium/r1cs.medium_')

        expected = {
            "primary_input": ["1", "0"],
            "aux_input": ["1", "1", "1"],
            "constraints": [
                [{"1": 1, "2": 1}, {"0": 1}, {"2": 1}],
                [{"2": 1}, {"3": 1}, {"3": 1}],
                [{"1": 1, "2": 1, "3": 1}, {"1": 1, "2": 1, "3": 1}, {"4": 1}]
            ]
        }

        self.assertEqual(json.dumps(expected), result)

    def test_export_pepper_to_json(self):

        f_name = 'satisfiable_pepper_'
        test_path = self.path + 'satisfiable/'

        export_pepper_to_json(test_path, f_name)

        with open(test_path + f_name + '.json') as file:
            result = json.load(file)

        self.assertEqual(self.expected_satisfiable, result)

    def test_satisfiable_pepper_output(self):
        f_name = 'satisfiable/satisfiable_pepper_'

        result = pepper_to_json(self.path, f_name)

        self.assertEqual(json.dumps(self.expected_satisfiable), result)

