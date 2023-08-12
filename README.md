# Lexer and Parser Documentation

## Lexer

The Lexer component is responsible for scanning the input file and generating tokens as the `scan` method is called. It also manages a symbol table for mapping standard strings read from the file to their respective token types. If an unidentified value is encountered, the Lexer returns a token of type `IDENTIFIER`, which needs to be managed by the Parser for type checking.

### Usage

```python
lexer = Lexer(input_file)
token = lexer.scan()  # Get the next token from the input file
```

### Symbol Table

The Lexer maintains a symbol table that maps standard strings to token types. This helps in identifying and categorizing different tokens. If a string is not found in the symbol table, it is treated as an identifier.

## Parser

The Parser component stores a linked list structure of Scope nodes. Each Scope node is initialized with a map to support entering soft scopes. The data element of a Scope is the symbol table. The `next` pointer points to the internal scope (e.g., Global -> Program -> Procedure), and the `previous` pointer facilitates exiting scopes.

### Usage

```python
parser = Parser()
parser.enter_scope()  # Enter a new scope
parser.exit_scope()   # Exit the current scope
```

### Scope Management

The Parser provides methods for entering and exiting scopes, both hard and soft, through the `Symbols` class.

#### Entering a Scope

```python
parser.enter_scope()  # Enter a new scope
```

#### Exiting a Scope

```python
parser.exit_scope()   # Exit the current scope
```

### Example

```python
parser = Parser()

# Entering a new scope
parser.enter_scope()
# ... Do operations within the scope ...
parser.exit_scope()

# Entering a soft scope
parser.enter_soft_scope()
# ... Do operations within the soft scope ...
parser.exit_scope()
```

## Symbols Class

The `Symbols` class provides methods for entering and exiting scopes, both hard and soft.

### Usage

```python
symbols = Symbols()
symbols.enter_scope(scope_data)   # Enter a new scope with optional scope_data
symbols.enter_soft_scope()        # Enter a soft scope
symbols.exit_scope()              # Exit the current scope
```

### Example

```python
symbols = Symbols()

# Entering a new scope
symbols.enter_scope(scope_data={'symbol_table': {...}})
# ... Do operations within the scope ...
symbols.exit_scope()
```

## Conclusion

The Lexer and Parser components work together to tokenize and structure the input file. The Lexer scans the file and generates tokens, while the Parser manages the scope hierarchy and provides methods for entering and exiting scopes. The Symbols class assists in scope management and maintains the symbol table for each scope.

- Lexer scans the file and generates next token whenever scan method is called.

- Lexer also managed symbol_table for mapping standard strings read from the file to their token type like PROGRAM_RW. In case of a unidentified value it returns IDENTIFIER type, which has to managed by Parser to check on type.

- Parser stores a linked list structure of Scope nodes.
Each node has default initializer setup to take a map to support entering soft scopes.
Data element of Scope is symbol_table.
pointer next gives the internal scope.(Global->Program->Procedure and so on)
pointer previous facilitates exiting scopes.

Methods for entering and exiting scopes(hard and soft) is provided by Symbols class
