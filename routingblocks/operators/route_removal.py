from typing import List

import routingblocks


class RouteRemoveOperator(routingblocks.DestroyOperator):
    def __init__(self, rng: routingblocks.Random):
        # Important: Do not use super()!
        routingblocks.DestroyOperator.__init__(self)
        self._rng = rng

    def can_apply_to(self, _solution: routingblocks.Solution) -> bool:
        return len(_solution) > 0

    def apply(self, evaluation: routingblocks.Evaluation, _solution: routingblocks.Solution, number_of_removed_vertices: int) -> List[
        int]:
        # Try to remove random routes
        removed_customers = []
        while len(_solution) > 0 and len(removed_customers) < number_of_removed_vertices:
            random_route_index = self._rng.randint(0, len(_solution) - 1)
            removed_customers.extend(x.vertex_id for x in _solution[random_route_index] if not x.vertex.is_depot)
            del _solution[random_route_index]
        return removed_customers

    def name(self) -> str:
        return "RouteRemoveOperator"
