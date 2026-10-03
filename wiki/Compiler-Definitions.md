# Compiler Definitions and Conditional Compilation

EDP-Clock defines no repository-owned compiler definitions or conditional-compilation switches. Public behaviour is configured through C++ Types, template parameters, Composition properties and selected providers rather than preprocessor configuration.

The new temporal-provenance Types and transition operations are unconditional C++20 surfaces. The three-target demo selects framework-specific Platform provider headers through its target source, not through EDP-Clock macros.

Externally supplied platform/framework macros are not EDP configuration knobs unless a provider repository explicitly defines them.

> Re-audited at `eda883aff0476faf8dbf8589af6c4ba059089c29`; no production or demo compiler definition was added.
