from typing import TypeVar, List
import random

T = TypeVar('T')


def distribute_randomly(sequence: List[T], num_subsequences: int, randgen=random.Random()) -> List[List[T]]:
    subsequences = [[] for _ in range(num_subsequences)]
    for item in sequence:
        subsequences[randgen.randint(0, len(subsequences) - 1)].append(item)
    return subsequences
