# Copyright (c) 2023 Patrick S. Klein (@libklein)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import pytest
from unittest.mock import Mock
import sys

import routingblocks
from routingblocks.operators import (
    first_move_selector,
    last_move_selector,
    nth_move_selector_factory,
    blink_selector_factory,
    random_selector_factory,
)


class MockRandom(routingblocks.Random):
    def __init__(self, values):
        super().__init__()
        self.values = values
        self.index = 0

    def uniform(self, start, end):
        value = self.values[self.index % len(self.values)]
        self.index += 1
        return value

    def randint(self, start, end):
        value = int(self.values[self.index % len(self.values)] * (end - start + 1)) + start
        self.index += 1
        return value


# Tests for first_move_selector with lists and iterators
@pytest.mark.parametrize("moves,expected", [
    ([1, 2, 3, 4, 5], 1),
    ((x for x in [1, 2, 3, 4, 5]), 1),
    (["a", "b", "c"], "a"),
    ((x for x in ["a", "b", "c"]), "a"),
])
def test_first_move_selector(moves, expected):
    assert first_move_selector(moves) == expected


# Tests for last_move_selector with lists and iterators
@pytest.mark.parametrize("moves,expected", [
    ([1, 2, 3, 4, 5], 5),
    ((x for x in [1, 2, 3, 4, 5]), 5),
    (["a", "b", "c"], "c"),
    ((x for x in ["a", "b", "c"]), "c"),
])
def test_last_move_selector(moves, expected):
    assert last_move_selector(moves) == expected


# Tests for nth_move_selector with lists and iterators
@pytest.mark.parametrize("n,moves,expected", [
    (3, [1, 2, 3, 4, 5], 3),
    (3, (x for x in [1, 2, 3, 4, 5]), 3),
    (2, ["a", "b", "c"], "b"),
    (2, (x for x in ["a", "b", "c"]), "b"),
])
def test_nth_move_selector(n, moves, expected):
    nth_selector = nth_move_selector_factory(n)
    assert nth_selector(moves) == expected


# Tests for blink_selector with mock random, lists and iterators
@pytest.mark.parametrize("probability,values,moves,expected", [
    (0.5, [0.2, 0.25, 0.5, 0.5 + sys.float_info.epsilon, 0.1], [1, 2, 3, 4, 5], 4),
    (0.5, [0.2, 0.25, 0.5, 0.5 + sys.float_info.epsilon, 0.1], (x for x in [1, 2, 3, 4, 5]), 4),
])
def test_blink_selector(probability, values, moves, expected):
    randgen = MockRandom(values)
    blink_selector = blink_selector_factory(probability, randgen)
    assert blink_selector(moves) == expected


# Tests for random_selector with mock random, lists, and iterators
@pytest.mark.parametrize("values,moves,expected", [
    ([0.2, 0.6, 0.8], [1, 2, 3, 4, 5], 2),
    ([0.2, 0.6, 0.8], (x for x in [1, 2, 3, 4, 5]), 2),
])
def test_random_selector(values, moves, expected):
    randgen = MockRandom(values)
    random_selector = random_selector_factory(randgen)
    assert random_selector(moves) == expected


# Tests for empty move sequences
@pytest.mark.parametrize("selector", [
    first_move_selector,
    last_move_selector,
    lambda moves: nth_move_selector_factory(1)(moves),
    lambda moves: blink_selector_factory(0.5, MockRandom([0.5]))(moves),
    lambda moves: random_selector_factory(MockRandom([0.5]))(moves)
])
def test_empty_moves(selector):
    with pytest.raises(AssertionError):
        selector([])
    with pytest.raises(AssertionError):
        selector(iter([]))
