# clang 笔记

## `class SourceLocation` 和 `class SourceManager`

  * 首先，文档上是这样写的：http://clang.llvm.org/docs/InternalsManual.html#the-sourcelocation-and-sourcemanager-classes

  * `SourceLocation` 所代表的是整个 translation unit 中的特定位置，它有一些特定的设计要求。

    * 非常小，目前的实现里是32bit的大小（现在实际上就是一个`unsigned`）

    * 其中最高一bit为macro bit，其余位表示一个offset， 此offset是offset到`class SourceManager`所维护的一系列buffers中的；
      这些buffers是怎么来的呢？实际上include一个文件会产生一个buffer；展开一次宏也会产生一个buffer，等等；
      macro bit非零时表示属于宏展开，零时表示为文件，更具体说明见下。

    * clang中的`SourceLocation`常常指向的是token的位置，而非char位置。见 http://clang.llvm.org/docs/InternalsManual.html#sourcerange-and-charsourcerange

      > Clang represents most source ranges by [first, last], where “first” and “last” each point to the beginning of their respective tokens. For example consider the SourceRange of the following statement:
      ```
      x = foo + bar;
      ^first  ^last
      ```
      > To map from this representation to a character-based representation, the “last” location needs to be adjusted to point to (or past) the end of that token with either
      > `Lexer::MeasureTokenLength()` or `Lexer::getLocForEndOfToken()`. For the rare cases where character-level source ranges information is needed we use the `CharSourceRange` class.

    * 另外`SourceLocaction.h`中`class SourceLocaction`前的[注释](https://github.com/llvm-mirror/clang/blob/release_36/include/clang/Basic/SourceLocation.h#L72)也很有意义，摘抄如下：

      ```c++
      /// \brief Encodes a location in the source. The SourceManager can decode this
      /// to get at the full include stack, line and column information.
      ///
      /// Technically, a source location is simply an offset into the manager's view
      /// of the input source, which is all input buffers (including macro
      /// expansions) concatenated in an effectively arbitrary order. The manager
      /// actually maintains two blocks of input buffers. One, starting at offset
      /// 0 and growing upwards, contains all buffers from this module. The other,
      /// starting at the highest possible offset and growing downwards, contains > /// buffers of loaded modules.
      ///
      /// In addition, one bit of SourceLocation is used for quick access to the
      /// information whether the location is in a file or a macro expansion.
      ///
      /// It is important that this type remains small. It is currently 32 bits wide.
      ```
  
  * `SourceManager`
    
    * 上述所说的buffers，指的是`SourceManager::LocalSLocEntryTable`以及`SourceManager::LoadedSLocEntryTable`这两个成员，
      它们实际上都是vector，其元素为`class SrcMgr::SLocEntry`即是所谓的buffer，一瞥相关的几个类：

      ```c++

      class SLocEntry {
        unsigned Offset;   // low bit is set for expansion info.
        union {
          FileInfo File;
          ExpansionInfo Expansion;
        };

      // ...
      };

      ```

      * `Offset` 中的最低bit如果非零则是`FileInfo`，零则是`ExpansionInfo`；其余位表示这个entry的offset。

      ```c++

      /// \brief Each ExpansionInfo encodes the expansion location - where
      /// the token was ultimately expanded, and the SpellingLoc - where the actual
      /// character data for the token came from.
      class ExpansionInfo {
      // Really these are all SourceLocations.
      
      /// \brief Where the spelling for the token can be found.
      unsigned SpellingLoc;
      
      /// In a macro expansion, ExpansionLocStart and ExpansionLocEnd
      /// indicate the start and end of the expansion. In object-like macros,
      /// they will be the same. In a function-like macro expansion, the start
      /// will be the identifier and the end will be the ')'. Finally, in
      /// macro-argument instantiations, the end will be 'SourceLocation()', an
      /// invalid location.
      unsigned ExpansionLocStart, ExpansionLocEnd;

      // ...
      }

      ```

      * `SpellingLoc` 怎么翻译呢？我这里翻译成‘字面位置’，表示这个宏展开的字面内容从何而来，例如对于宏展开时：

        ```c++

        // Function-like macro
        #define MIN(x,y) ((x)>(y)?(y):(x))
        //               ^SpellingLoc

        // Object-like macro
        #define ZERO (0)
        //           ^SpellingLoc
        ```

      * `ExpansionLocStart` 和 `ExpansionLocEnd` 指向的是展开的位置，例如：

        ```c++

        // Function-like macro expansion
        int foo() {
            return MIN(1000001, 1000002);
        //         ^ExpansionLocStart  ^ExpansionLocEnd
        }

        // Object-like macro expansion
        int bar() {
            return ZERO;
        //         ^ExpansionLocStart
        //         ^ExpansionLocEnd
        }
        ```

        * **Macro** 有两种，一种是**Function-like**，带参数（参数可以为0）；另外一种是**Object-like**，不带参数；
          前者展开时，`ExpansionLocEnd`指向最后右括弧，后者展开时，`ExpansionLocEnd`跟`ExpansionLocStart`都指向
          同一个地方。

    * 上些例子比较直观：    
      
      ```c
      // nest_expansion.c
      #define min(x, y) ((x) > (y) ? (y) : (x))
      #define concat_with_in(x) x ## in
      
      concat_with_in(m)(100, 101);
      ```

      * 以下是用自己的方法打印出`LocalSLocEntryTable` 中的每一条记录，

        ```
        Number of LocalSLocEntryTable: 12
        0: offset(0) type(E) range([0 0], [0 0]) spelling([0 0])
        1: offset(2) type(F) orig(name(nest_expansion.c) size(126)) contents(name(nest_expansion.c) size(126)) 
        2: offset(129) type(F) 
        3: offset(10245) type(Ebf) range([1 97], [1 113]) spelling([1 88])
        4: offset(10253) type(Ea) range([3 0], [3 0]) spelling([1 112])
        5: offset(10255) type(F) 
        6: offset(14316) type(Ebf) range([3 0], [3 5]) spelling([5 2])
        7: offset(14320) type(Ebf) range([6 0], [1 123]) spelling([1 38])
        8: offset(14344) type(Ea) range([7 2], [7 2]) spelling([1 115])
        9: offset(14348) type(Ea) range([7 8], [7 8]) spelling([1 120])
        10: offset(14352) type(Ea) range([7 14], [7 14]) spelling([1 120])
        11: offset(14356) type(Ea) range([7 20], [7 20]) spelling([1 115])
        ```

        * 第一个数字是第几条记录，offset是`SLocEntry::Offset`，type(F)表示是一个`FileInfo`，type(Exx)表示是一个`ExpansionInfo`；
          
          * Exx 的 xx 中如果有 b 则表示是 `isMacroBodyExpansion`
          * Exx 的 xx 中如果有 a 则表示是 `isMacroArgExpansion`
          * Exx 的 xx 中如果有 f 则表示是 `isFunctionMacroExpansion`
  
        * 对于`FileInfo`来说，如果有文件名的则也打印出来，否则这个`FileInfo`一般是虚拟的文件（例如用宏操作符 `#`或 `##`产生的）
  
        * 对于`ExpansionInfo`来说，range就是`ExpansionLocStart`和`ExpansionLocEnd`，spelling则是`SpellingLoc`；
  
        * `SourceLocation`则用'[]'来表示，里头有两个部分，第一个指向第几条记录，
          第二个则是相对那条记录的相对offset。

      * 详细的解释：
        
        * 0号记录表示 invalid，初始化时即插入，其size为1（见`SourceManager::clearIDTables`) 其offset为0。

          > 0: offset(0) type(E) range([0 0], [0 0]) spelling([0 0])

        * 1号记录明显就是主文件 nest_expansion.c，长度是126个字节，其offset为2，原来每一条记录它都会在
          它的长度上加上1，原因见`SourceManager::createFileID`中最后的注释：

          ```c++
          // We do a +1 here because we want a SourceLocation that means "the end of the
          // file", e.g. for the "no newline at the end of the file" diagnostic.
          NextLocalOffset += FileSize + 1;
          ```

          * 即加多一个字节，使得 eof 也是一个有效的 `SourceLocation`，所以实际每条记录的offset是这样算出来的：

            * offset = prev_offset + prev_file_size + 1

          * 故这条记录的 offset = 0 + 1 + 1 = 2，而第三条记录的 offset = 2 + 126 + 1 = 129

        * *2号记录没有文件名，是什么我还没弄清楚，但offset一下子增加了很多，好像是创建了一个挺大的虚拟文件*。

        * 3号记录是一个**Function-like macro body expansion** (_Ebf_)，它的range指向1号记录的offset[97,113]
          1号记录就是nest_expansion.c，用python跑一下：

          ```
          >>> s = open('nest_expansion.c').read()
          >>> s[97:113+1]       # expansion range 指向宏展开的地方
          'concat_with_in(m)'
          >>> s[88:88+7]        # spelling 指向宏定义的地方
          'x ## in'
          ```

          * 这里的7是用上述算法倒推而出：size = next_offset - offset - 1 = 10253 - 10245 - 1 = 7
            实际上 `SourceManager::getFileIDSize` 也做同样的事情：

            ```
            return NextOffset - Entry.getOffset() - 1;
            ```

        * 4号记录是一个**Macro arg expansion** (_Ea_)，它是3号记录的offset为0的地方的展开, 3号记录的
          字面内容为 `x ## in`，故offset为0即是`x`；而其字面位置为[1 112]指向主文件：

          ```
          >>> s[112:112+(10255-10253-1)]
          'm'
          ```

          * 所以其字面内容为`m`


        

          
      


