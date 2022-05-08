import dataclasses as dc


@dc.dataclass
class Parser:
    data: str
    pointer: int

    def __post_init__(self):
        self.data_size = len(self.data)

    def _read(self):
        return self.data[self.pointer]

    def _forwards_until(self, n):
        while self.pointer < self.data_size:
            if n == self._read():
                break

    def _forwards_until_closure(self, c):
        while self.pointer < self.data_size:
            if c(self.read()):
                break

    def normalize(self):
        pass


"(a! hello (a! hi) world)"

# tokenize into nodes
# normalize nodes
# turn nodes into string again
