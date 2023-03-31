from typing import Iterable, Callable, TypeVar
from collections.abc import Sequence

import routingblocks

T = TypeVar('T')
MoveSelector = Callable[[Iterable[T]], T]
import routingblocks as alns


def first_move_selector(moves: Iterable[T]) -> T:
    move = next(iter(moves), None)
    assert move is not None, "Unable to select a move from an empty sequence"
    return move


def last_move_selector(moves: Iterable[T]) -> T:
    if isinstance(moves, Sequence):
        return moves[len(moves) - 1]
    move = None
    # Exhaust the iterator
    for move in moves:
        pass
    assert move is not None, "Unable to select a move from an empty sequence"
    return move


def nth_move_selector_factory(n: int) -> MoveSelector[T]:
    assert n > 0

    def select(moves: Iterable[T]) -> T:
        if isinstance(moves, Sequence):
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


def blink_selector_factory(blink_probability: float, randgen: alns.Random) -> MoveSelector[T]:
    assert 0 <= blink_probability <= 1

    def select(moves: Iterable[T]) -> T:
        move = None
        for move in moves:
            if randgen.uniform(0, 1) <= blink_probability:
                continue
            break
        assert move is not None, "Unable to select a move from an empty sequence"
        return move

    return select


def random_selector_factory(rangen: routingblocks.Random):
    def select(moves: Iterable[T]) -> T:
        # TODO Improve
        if not isinstance(moves, Sequence):
            moves = [*moves]
        pos = rangen.randint(0, len(moves) - 1)
        return moves[pos]

    return select
