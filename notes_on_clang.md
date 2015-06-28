# clang 笔记

## class SourceLocation

首先，文档上是这样写的：http://clang.llvm.org/docs/InternalsManual.html#the-sourcelocation-and-sourcemanager-classes

  * SourceLocation 所代表的是整个 translation unit 中的特定位置，它有一些特定的设计要求。

    * 非常小，目前的实现里是32bit的大小（现在实际上就是一个`unsigned`）

    * 其中最高一bit为macro bit，其余位表示一个offset， 此offset是`class SourceManager`所维护的一个'buffer'中的offset；
      此'buffer'的内容包含了这个translation unit中各个被包含进来的文件以及各种宏展开（macro expansion）所产生的内容。
      （macro bit非0时表示属于宏展开，0时表示为文件）。

    * clang中的`SourceLocation`常常指向的是token的位置，而非char位置。见 http://clang.llvm.org/docs/InternalsManual.html#sourcerange-and-charsourcerange

      > Clang represents most source ranges by [first, last], where “first” and “last” each point to the beginning of their respective tokens. For example consider the SourceRange of the following statement:
      ```
      x = foo + bar;
      ^first  ^last
      ```
      > To map from this representation to a character-based representation, the “last” location needs to be adjusted to point to (or past) the end of that token with either
      > Lexer::MeasureTokenLength() or Lexer::getLocForEndOfToken(). For the rare cases where character-level source ranges information is needed we use the CharSourceRange class.

    

