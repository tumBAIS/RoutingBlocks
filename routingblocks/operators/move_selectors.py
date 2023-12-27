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

from typing import Iterable, TypeVar, Protocol, Generic
from collections.abc import Sequence

import routingblocks

T = TypeVar('T')


class MoveSelector(Protocol[T]):
    """
    A move selector selects a move from a sequence of moves.
    """

    def __call__(self, moves: Iterable[T]) -> T:
        """
        Selects a move from the sequence of moves.

        :param moves: The sequence of moves.
        :return: The selected move.
        """
        ...


def first_move_selector(moves: Iterable[T]) -> T:
    """
    Selects the first move in the sequence.

    :param moves: The sequence of moves.
    :return: The first move in the sequence.
    """
    move = next(iter(moves), None)
    assert move is not None, "Unable to select a move from an empty sequence"
    return move


def last_move_selector(moves: Iterable[T]) -> T:
    """
    Selects the last move in the sequence.

    :param moves: The sequence of moves.
    :return: The last move in the sequence.
    """
    if isinstance(moves, Sequence):
        assert len(moves) > 0, "Unable to select a move from an empty sequence"
        return moves[len(moves) - 1]
    move = None
    # Exhaust the iterator
    for move in moves:
        pass
    assert move is not None, "Unable to select a move from an empty sequence"
    return move


def nth_move_selector_factory(n: int) -> MoveSelector[T]:
    """
    Creates a move selector which selects the nth move in the sequence.

    :param n: The index of the move to select.
    :return: A move selector which selects the nth move in the sequence.
    """
    assert n > 0

    def select(moves: Iterable[T]) -> T:
        if isinstance(moves, Sequence):
            assert len(moves) > n, "Unable to select a move from an empty sequence"
            return moves[n - 1]
        remaining = n
        move = None
        # Returns even if there are fewer than n moves
        for move in moves:
            remaining -= 1
            if remaining == 0:
                break
        assert move is not None, "Unable to select a move from an empty sequence"
        return move

    return select


def blink_selector_factory(blink_probability: float, randgen: routingblocks.Random) -> MoveSelector[T]:
    """
    Creates a move selector which selects the first move of the sequence with probability :math:`(1-p)`, the second move with
    probability :math:`(1-p)^2`, where :math:`p` is the blink probability, and so on.

    :param blink_probability: The probability of blinking.
    :param randgen: The random number generator.
    :return: The configured move selector.
    """
    assert 0 <= blink_probability <= 1

    def select(moves: Iterable[T]) -> T:
        move = None
        for move in moves:
            if randgen.uniform(0, 1) <= blink_probability:
                continue
            break
        assert move is not None, "Sequence was exhausted before a move was selected"
        return move

    return select


def random_selector_factory(rangen: routingblocks.Random):
    """
    Creates a move selector which selects a random move from the sequence.

    :param rangen: The random number generator.
    :return: The configured move selector.
    """

    def select(moves: Iterable[T]) -> T:
        if not isinstance(moves, Sequence):
            moves = [*moves]
        assert len(moves) > 0, "Unable to select a move from an empty sequence"
        pos = rangen.randint(0, len(moves) - 1)
        return moves[pos]

    return select
