"""Provides a Macro class."""
import dataclasses as dc
import typing as t
import math


@dc.dataclass
class Arity:
    start: float = -math.inf
    stop: float = math.inf

    def __contains__(self, num):
        return self.stop >= num >= self.start

    @classmethod
    def any(cls):
        return cls(-math.inf, math.inf)

    @classmethod
    def min(cls, n):
        return cls(start=n)

    @classmethod
    def max(cls, n):
        return cls(stop=n)

    @classmethod
    def exactly(cls, n):
        return cls(n, n)


@dc.dataclass
class Macro:
    """Format some code."""

    names: t.List[str]
    fmt: str = None
    arity: Arity = Arity.any()
    func: t.Callable = None

    def __post_init__(self):
        """Initialize the arity."""

        if isinstance(self.names, str):
            self.names = [self.names]

        if isinstance(self.arity, (int, float)):
            self.arity = Arity.exactly(self.arity)

        assert not (self.fmt is None and self.func is None)
        self._func = self.fmt.format if self.func is None else self.func

    def _format(self, expression: list) -> str:
        """Format an expression."""
        return self._func(args=expression[1:])

    def format_or_throw(self, expression: list) -> str:
        """Format an expression or throw BadMacroArity."""
        nargs = len(expression) - 1
        if nargs not in self.arity:
            raise BadMacroArity(nargs, self)
        return self._format(expression)


@dc.dataclass
class BadMacroArity(BaseException):
    """Thrown on incorrect macro arity."""

    n: int
    macro: Macro

    def __repr__(self):
        """Get representation."""
        m = self.macro
        macro_name = " / ".join(map("`{}`".format, m.names))
        if (m.arity.start - m.arity.stop) < 1:
            num = str(m.arity.start)
        else:
            num = f"{m.arity.start}-{m.arity.stop}"

        return f"Macro {macro_name} expected {num} arguments, but recieved {self.n}."
