import re
import sys
from collections import namedtuple

N_INPUTS = 4
keywords = ["unit", "as", ":"]
Token = namedtuple("Token", ["type", "content"])
synth_module_regex = re.compile(r"([a-zA-Z][a-zA-Z0-9_]*)(<.*>)?(\(.*\))?")
unit_regex = re.compile(r"unit +([a-zA-Z0-9_]+):")


class Unit:
    def set_name(self, name: str):
        self.__name = name

    def get_name(self):
        return self.__name

    def get_module_id(self, alias: str):
        return self.__alias_to_index.get(alias, None)

    def add_module(
        self, name: str, parameters: list, sources: list, alias: str | None = None
    ):
        if name.startswith("Mod"):
            parameters.append(str(self.__next_mod_uid))
            self.__next_mod_uid += 1
        self.__modules.append((name, parameters, sources))
        module_id = len(self.__modules) - 1
        if alias:
            self.__alias_to_index[alias] = module_id
        return module_id

    def dump_and_clear(self):
        if not self._is_empty():
            self._dump()
            self._clear()

    def _dump(self):
        print(f"SynthUnit {self.__name} = CompileSynthUnit<")
        for i, (name, params, sources) in enumerate(self.__modules):
            print("SynthUnitNode{")
            print(f".unit = {name}", end="")
            if params:
                print("<" + ", ".join(params) + ">", end="")
            print(",")
            print(
                f".inputs = {{{', '.join([str(sources[i]) if i < len(sources) else 'kSynthNoInput' for i in range(N_INPUTS)])}}},"
            )
            if i == len(self.__modules) - 1:
                print("}")
            else:
                print("},")
        print(">;")

    def _clear(self):
        self.__modules = []
        self.__name = ""

    def _is_empty(self):
        return not (
            bool(self.__alias_to_index) or bool(self.__modules) or bool(self.__name)
        )

    __alias_to_index = {}
    __modules = []
    __name = ""
    __next_mod_uid = 0


def is_valid_variable_name(name: str):
    return bool(re.match(r"[a-zA-Z0-9_]+", name)) and name[0].isalpha()


def is_number(s: str):
    try:
        float(s)
        return True
    except ValueError:
        return False


def is_integer(s: str):
    return s.isdecimal() or (s[0] == "-" and s[1:].isdecimal())


def tokenize_module(module: str):
    match = re.fullmatch(synth_module_regex, module)
    if not match:
        raise Exception(f'"{module}" does not match module pattern.')
    name = match.group(1)
    params = []
    sources = []

    if not is_valid_variable_name(name):
        raise Exception(f'"{name}" is not a valid name.')

    if match.group(2):
        params_str = match.group(2)
        for item in params_str[1:-1].split(","):
            item = item.strip()
            if is_number(item):
                params.append(Token("number", item))
            else:
                raise Exception(f'"{item}" is not a number.')

    if match.group(3):
        sources = list(tokenize_sources(match.group(3)[1:-1]))

    return Token("module", (name, params, sources))


def tokenize_sources(sources: str):
    for item in sources.split(","):
        item = item.strip()
        if is_number(item):
            yield Token("number", item)
        elif re.fullmatch(r"\[-?[0-9]+\]", item):
            yield Token("source_id", int(item[1:-1]))
        elif is_valid_variable_name(item):
            yield Token("var_name", item)
        elif re.fullmatch(synth_module_regex, item):
            yield tokenize_module(item)
        else:
            raise Exception(f'Invalid source argument: "{item}".')


def process_and_add_module(unit: Unit, module: Token, alias: str | None = None):
    name, params, sources = module.content
    source_ids = [process_source_and_get_id(unit, source) for source in sources]
    param_strs = [token.content for token in params]
    return unit.add_module(name, param_strs, source_ids, alias)


def process_source_and_get_id(unit: Unit, source: Token):
    if source.type == "source_id":
        return int(source.content)
    elif source.type == "var_name":
        mod_id = unit.get_module_id(source.content)
        if mod_id is None:
            # Assuming var_name refers to an existing unit name
            return unit.add_module(name=source.content, parameters=[], sources=[])
        else:
            return mod_id
    elif source.type == "number":
        return unit.add_module(name="modConst", parameters=[source.content], sources=[])
    elif source.type == "module":
        return process_and_add_module(unit, source.content)
    raise Exception(
        f'Invalid module source was encountered while processing unit "{unit.get_name()}"'
    )


if __name__ == "__main__":
    current_unit = Unit()
    for line in sys.stdin:
        line = line.strip()
        if re.fullmatch(unit_regex, line):
            current_unit.dump_and_clear()
            name = re.fullmatch(unit_regex, line).group(1)  # type: ignore
            current_unit.set_name(name)
        elif re.match(synth_module_regex, line):
            match = re.match(synth_module_regex, line)
            appendix = line[len(match.group(0)):]  # type: ignore
            alias = None
            if appendix:
                appendix_match = re.fullmatch(r" +as +([a-zA-Z0-9_]+) *", appendix)
                if not appendix_match:
                    raise Exception(
                        f'End of line or alias was expected after module description in line: "{line}".'
                    )
                alias = appendix_match.group(1)
                if not is_valid_variable_name(alias):
                    raise Exception(f'"{alias}" is not a valid alias name.')
            module_str = match.group(0)  # type: ignore
            process_and_add_module(current_unit, tokenize_module(module_str), alias)
        elif line and line[0] != "#":
            sys.stderr.write(f'WARNING: skipping non-empty line: "{line}".\n')
    current_unit.dump_and_clear()
