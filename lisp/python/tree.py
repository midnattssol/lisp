#!/usr/bin/env python3.10
"""An implementation of a basic tree structure."""
import collections as col
import itertools as it
import enum
import typing as t

import more_itertools as mit

# ====| GLOBALS |====

Q, W, E, R, T = map(t.TypeVar, "QWERT")
TrieMatch = enum.Enum("TrieMatch", "VALID INVALID NONE")
TrieNode = col.namedtuple("TrieNode", ["string", "terminal"])

# ====| CLASSES |====


class Tree(t.Generic[T]):
    """A class implementing a tree structure."""

    def __init__(
        self,
        /,
        nodes: t.Optional[t.List[T]] = None,
        depths: t.Optional[t.List[int]] = None,
    ):
        self.nodes = nodes if nodes is not None else list()
        self.depths = depths if depths is not None else list()

        if len(self.nodes) != len(self.depths):
            raise ValueError(
                f"Nodes {self.nodes} and depths {self.depths} must be of the same length."
            )

    def __repr__(self):
        out = ""
        for node, depth in self:
            out += "  " * depth + repr(node) + "\n"
        return out.rstrip()

    def __bool__(self):
        return bool(self.nodes)

    def __len__(self):
        return len(self.nodes)

    def __iter__(self):
        return (self[index] for index in range(len(self.nodes)))

    def __getitem__(self, key):
        return self.nodes[key], self.depths[key]

    def set_parent(self, value: T) -> None:
        """Sets the parent of the tree."""
        if not self:
            self.nodes.append(value)
            self.depths.append(0)
        else:
            self.nodes[0] = value

    def index(self, node: T) -> int:
        """Get the index of the node."""
        return self.nodes.index(node)

    def insert(self, index: int, child_node: T) -> None:
        """Add a child to the node with index `index`."""
        parent_depth = self.depths[index]
        number_of_children = mit.ilen(self.child_indices_of(index))
        new_pos = index + number_of_children + 1

        self.nodes.insert(new_pos, child_node)
        self.depths.insert(new_pos, parent_depth + 1)

    def pop(self, index: int, *, include_children: bool = False) -> None:
        """Remove a child by index. If include_children is True, the children of this node are also removed."""
        if include_children:
            children = self.child_indices_of(index)
            number_of_items = 1 + mit.ilen(children)
            output = [
                (self.nodes.pop(index), self.depths.pop(index))
                for _ in range(number_of_items)
            ]
            return output

        return self.nodes.pop(index), self.depths.pop(index)

    def direct_child_indices_of(self, index: int) -> t.Iterable[int]:
        """Returns direct child indices of the index."""
        return filter(
            lambda x: (self.depths[x] - 1) == self.depths[index],
            self.child_indices_of(index),
        )

    def child_indices_of(self, index: int) -> t.Iterable[int]:
        """Get child indices of the node with index `index`."""
        parent_depth = self.depths[index]
        children = it.takewhile(lambda x: x > parent_depth, self.depths[index + 1 :])
        return range(index + 1, index + mit.ilen(children) + 1)

    def children_of(self, index: int) -> t.Iterable[T]:
        """Get children of the node with index `index`."""
        return map(self.nodes.__getitem__, self.child_indices_of(index))

    def parent_index_of(self, index: int) -> t.Optional[int]:
        """Get the parent node of the index. Returns None if the index is top-level."""
        depth = self.depths[index]
        depths_up_to = self.depths[:index][::-1]
        siblings_and_niblings = it.takewhile(lambda x: x >= depth, depths_up_to)
        distance_to_parent = mit.ilen(siblings_and_niblings) + 1

        return (
            index - distance_to_parent if (index - distance_to_parent) != -1 else None
        )

    def index_of_grandparent_of_depth_of(
        self, depth: int, index: int
    ) -> t.Optional[int]:
        """Return the index of the first node in direct line of succession
        to the node of index `index` with depth `depth`."""
        parent_idx = index
        while self.depths[parent_idx] != depth:
            parent_idx = self.parent_index_of(parent_idx)
            if parent_idx is None:
                break
        else:
            return parent_idx

        return None

    def subtree_of(self, index: int):
        """Get the subtree starting with the node at index `index`."""
        child_indices = [index, *self.child_indices_of(index)]
        return Tree(
            nodes=[self.nodes[i] for i in child_indices],
            depths=[self.depths[i] - self.depths[index] for i in child_indices],
        )

    def nodes_and_parents(self) -> t.List[t.Tuple[t.Union[T, int]]]:
        """Yield each successive node in the tree and its parents."""
        stack = []
        for item, depth in self:
            stack = [*it.takewhile(lambda a, b=depth: a[1] < b, stack), (item, depth)]
            yield stack

    def top_level(self) -> t.Iterable[int]:
        return (i for i, (_, depth) in enumerate(self) if not depth)


class Trie(Tree[TrieNode]):
    """Trie implementation based on the Tree class.

    See https://en.wikipedia.org/wiki/Trie for more information.
    """

    @classmethod
    def from_words(cls, words: t.List[str]):
        """Generate a trie from some words."""
        words = sorted(words)
        nodes = [""]
        depths = [0]

        for word in words:
            for i, letter in enumerate(word, 1):
                nodes.append(letter)
                depths.append(i)

        trie = Trie(nodes, depths)
        trie._clean_direct_children()

        words = set(words)

        for i, node in enumerate(trie):
            first_child, _ = mit.spy(trie.children_of(i))
            trie.nodes[i] = TrieNode(string=trie.nodes[i], terminal=not first_child)

        return trie

    def matches(self, value: str) -> bool:
        """Returns a TrieMatch object corresponding to whether the value matches the trie or not."""
        val = ""

        for (item, depth) in self:
            val = val[: depth - 1] + item.string
            if value == val:
                return TrieMatch.INVALID if not item.terminal else TrieMatch.VALID

        return TrieMatch.NONE

    def match_longest(self, value: str, /, default: str = "") -> str:
        """Matches the longest possible value against a trie.
        Returns the optional keyword-only value `default` if no matches are found."""

        matches = [TrieMatch.INVALID]
        value_len = len(value)
        while matches[-1] != TrieMatch.NONE and value_len >= len(matches):
            matches.append(self.matches(value[: len(matches)]))

        valid_matches = [TrieMatch.VALID == i for i in matches]

        if not any(valid_matches):
            return default
        else:
            return value[: max(i for i, match in enumerate(valid_matches) if match)]

    def _clean_direct_children(self, index: int = 0) -> None:
        """Recursively remove unneccessary nodes."""
        direct_children = [*self.direct_child_indices_of(index)]
        if not direct_children:
            return

        child_value_counter = col.Counter(self.nodes[i] for i in direct_children)
        non_unique_child_values = [k for k, v in child_value_counter.items() if v > 1]
        should_remove = []
        seen = set()

        for i in direct_children:
            if self.nodes[i] in non_unique_child_values:
                if self.nodes[i] not in seen:
                    seen.add(self.nodes[i])
                else:
                    should_remove.append(i)

        for i in reversed(should_remove):
            self.pop(i)

        for child in reversed(list(self.direct_child_indices_of(index))):
            self._clean_direct_children(child)
