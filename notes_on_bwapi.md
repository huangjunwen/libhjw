#summary BWAPI 笔记

BWAPI 2.4 的版本号是 1689, 以下均基于此, 现在 trunk 上的代码似乎在大改...

  * 星际地图的原点在左上角, x 坐标从左到右增加, y 坐标从上到下增加.


  * BWAPI 的主体逻辑实现在 [bwapi\source\bwapi\GameImpl.cpp 的 GameImpl::update](http://code.google.com/p/bwapi/source/browse/trunk/bwapi/BWAPI/Source/BWAPI/GameImpl.cpp?r=1689#657) 中, 在这个函数中可以看到 onPlyerLeft/onNukeDetect/onStart/onFrame 这几个 callback 是在什么情况下被调用的.


  * Map max (mm) is a term used to describe the conditions of a map when the maximum alotted units (1700: inclusive of units, buildings, and some spells) for that entire map is met.  ref http://starcraft.wikia.com/wiki/Map_max

  * 星际中最大的单位数量是 1700, 这从 BWAPI 中的一个常量也可以看到 `UNIT_ARRAY_MAX_LENGTH`, 游戏之初即初始化了一个数组 (BWDATA\_UnitNodeTable), 单位创建就从中取出, 移除就释放.

  * BWAPI 同时也维护多一个数组 `unitArrayCopyLocal`, 这个数组是 `BWDATA_UnitNodeTable` 的拷贝(每次调用 update 都会 memcpy 一次), 而 BWAPI::Unit 的实现 BWAPI::UnitImpl 则是再在这个之上的一层封装 (拷贝 `unitArrayCopyLocal` 出来应该是为了防止 starcraft 更新了状态而 BWAPI 没有同步, UnitImpl 内部指向的地址出现不一致的情况).

  * 关心的一点: BWAPI 返回的 Unit 指针是否一直有效, 还是指向一个 slot? 答案是前者, 即使一个单位被干掉之后, 指针还是一直有效的, 只是其上的标志位改变了, 直到游戏结束才会回收这些内存.
```
    GameImpl::onUnitDeath
    // ...
    this->units.erase(unit);
    deadUnits.push_back(unit);
    unitArray[index] = new UnitImpl(&BW::BWDATA_UnitNodeTable->unit[index],
                                        &unitArrayCopyLocal->unit[index],
                                        (u16)index);            // new 一个新的出来替换原先的 slot
    // ...
   
    GameImpl::onGameEnd()
    // ...
    foreach (UnitImpl* d, this->deadUnits)
    delete d;
    // ...
```