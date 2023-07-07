# Learn_GAS



## GAS 介绍

GAS 的优缺点

| 优点 | 缺点 |
| --- | --- |
| 灵活易扩展, 可实现复杂的技能流程 | 大量的概念和类, 学习曲线陡峭 |
| 支持联机复制和预测回滚 | 重度C++, BP不友好 |
| 易于团队协作和项目复用 | 需要按照框架定义一堆才能开始启动 |
| 细粒度思考实现单个动作逻辑 | GAS的网络服务必须配合DS |
| 数据驱动数值配置 | 实践演化框架, 可选插件, 文档不足 |
| 已经帮你处理复杂流程麻烦逻辑 | 要求技术功底高, 源码Debug能力强 |
| 适用于: 人多-大项目-多技能-联机-技术强-重表现 | 不适用于: 人少-小项目-少技能-单机-技术弱-弱表现 |

如何自己实现一个易于扩展的技能系统? 

- 逻辑部分:
  - 技能的获得和释放
  - 触发判断的条件
  - Buff系统
- 试听部分:
  - 动作动画
  - 特效
  - 声效
- 数据部分:
  - 数值计算
  - 数据配置
- 考虑网络联机

设计模式的本质是: 找到变化, 封装变化

由于上述这些技能的需求, 从而促成了 GAS 的诞生

| GAS 类 | 简称 |
| --- | --- |
| UAbilitySystemComponent | ASC |
| UGameplayAbility | GA |
| UGameplayEffect | GE |
| UGameplayCueNotify | GC |
| FGameplayAttribute | Attribute |
| FGameplayTag | Tag |
| UGameplayTask | Task |
| FGameplayEventData | Event |

- 谁可以释放技能？ 持有 ASC 组件的 Actor
- 如何编写技能逻辑？ 继承 GameAbility 
- 如何技能效果？ 继承 GameEffect (属性修改、增减Buff, 并不是特效)
- 技能改变的什么属性？ 使用 GamePlayAttribute 属性系统
- 技能释放的条件？ 使用 GameplayTag 进行条件判断
- 技能的视角表现效果？ 使用 GameplayCue
- 技能的长时行动？ 使用 GameplayTask 
- 技能消息事件？ 使用 GameplayEvent

### GameplayTags

GameplayTags 游戏标签

- 是层次化的字符串标签 "A.B.C"
- 轻量级 `FName` , 可附加到各类上做搜索条件: `GameplayEffect`, `GameplayAbility`, `GameplayCue`, `GameplayEventData`
- 整体所有 `Tag` 构成一颗 `Tag` 树

![](Images/017.png)

![](Images/018.png)

可以在子标签上进行精确匹配, 也可以在父标签上进行总类筛选

子标签: `Ability.Melee.Close`; 父标签: `Ability.Melee`

比如当角色存在 `Ability.Melee.Close` 和 `Ability.Melee.Far` 等 Melee 下的 Tag 时技能不生效, 那么只用配置成 `Ability.Melee` 即可。 这就是父标签可以做的总类筛选

![](Images/019.png)

所有的 `GameplayTag` 都可以在 Project `Settings` 中找到

使用 GameplayTag 相比代码中写死 Bool、Enum 更加灵活

![](Images/020.png)

### GameplayEffect 

技能效果

- 决定一个技能的逻辑效果
- 可以配置: 类型、修改器、周期、应用需求、溢出处理、过期处理、显示处理、Tags条件、免疫、堆叠、能力赋予
- Effect 是修改 Attribute 的唯一通道

![](Images/021.png)

GE 一般通过 `UGameplayAbility` 的 Aplly 接口, 以 `UGameplayEffect` 为模板生成 `FGameplayEffectSpec` 实例化对象, `FGameplayEffectSpec` 对应一个 `FActiveGameplayEffect`, 然后通过组合到 `FActiveGamplayEffectsContainer` 容器中, 存储到 `UAbilitySystemComponent` 中

### GampelayCue 

技能特效

- 决定一个技能的视觉效果
- 可全局配置 `Tag-Handler` 的映射
- 可通过 `Effect` 触发, 也可以手动触发
- `Static`: 一次性的; `Actor`: 持久的创建在场景中
- 可重载: `OnActive` 、 `WhileActive` 、 `Executed` 、 `Removed` 来触发粒子特效或者音效等

![](Images/022.png)

如上图可见, GC 一般通过 `GA` 指定 `Executed/Add` 或者 GE 配置 `Cues` 来触发;触发后生成的实例存储在 FGameplayCueObjectLibrary 里的对象库中, 以提供后续的复用, 介绍触发时的延迟

GC 的触发分为两种情况: `Static` 和 `Actor`。`Static Cue` 直接在 `CDO` 上调用, 这意味着不要在 GC 中存储一些状态信息, 因为它不会生成新的对象; `Actor Cue` 触发 `Spawn` 生成新的实例, 这些生效的 GC 最终也是在 ASC 里引用保存

### GameAbility

游戏能力(技能)

- 能力是很广义的抽象 比如被打就是一种能力 没有被打的能力就不会被被打
- 不要把基础移动, 射线检测, UI交互当作能力
- 在不同的 Actor 上学习、取消、释放不同能力构成了主题逻辑
- 主要逻辑都写在 GA 里面
- 可重载: ActivateAbility, CommitAbility, CancelAbility, EndAbility

![](Images/023.png)

GA 一般是在外部被 `Owner Actor` 触发, 通过调用 `TryActivateAbility`

如果 GA 自身不需要保存状态, 则可以选择在 CDO 上直接调用; 如果需要每次实例化都跟踪不同的状态, 也可以选择每次生成新的实例

GA 实例化之后就是 `FGameplayAbilitySpec` , 可以通过配置 `Level` 去传递给 GA 触发的 GE, 从而触发不同等级的效果

这些激活的 GA 最后都会保存在 ASC 中

### GameplayTask

游戏异步任务

> 比如: 等待 `montage` 的播放完毕、 等待 AI 移动到指定位置

- 执行异步任务的框架
- 可被单独使用-> Task 是 UE 的基础模块, 可以独立与 GAS 之外运行
- 可用于异步长时间动画动作的触发和等待
- 已经预先实现好一系列 Tasks
- 可重载: `Activate、` `TickTask`

![](Images/024.png)

`Abilities` 插件引用 Task 模块的时候其实也是从 `GameplayTask` 里继承下来自定义子类, 这个子类多了一个 GA 的引用

这些 Task 也都是在 ASC 中执行, 所以 ASC 也是继承于 `UGameplayTasksComponent` 的

### 游戏事件

![](Images/025.png)

GAS 提供手动触发游戏事件

- 事件靠 Tag 识别, 可写代 Payload 数据
- 可触发技能
- 在另一端可等待具体 Tag 事件触发

![](Images/026.png)

GA 在 Wait GameplayEvent 的时候会在 ASC 内部记录一个回调, 通过 Tag 来映射识别; 而当外部别的 GA 调用 Send GameplayEvent 的时候, 会用 Event 的 Tag 在 ASC 的映射表中搜索, 如果搜索到就会触发对应的回调

### AbilitySystemComponent

游戏技能组件

- ASC 负责管理协调其他部分: Ability、 Effect、 Attribute、 Task、 Event...
- ASC 是技能系统运行的核心
- 拥有 ASC 的 Actor 才拥有释放技能的能力
- ASC 放在 Pawn 还是 PlayerState 上, 这是一个问题

如果是联机游戏, 推荐放在 PlayerState 上, 因为可以同步给客户端; Pawn 则有可能死亡, 重生后丢失状态

比如: dota 的英雄死亡会被Destroy掉, 如果 ASC 在 Pawn 上那么所有的数据也就丢失了, 此时角色重生之后技能的 CD 信息也就没有了

![](Images/027.png)

从上图的继承关系可以发现, ASC 继承自 `UGameplayTasksComponent`, 所以它具有执行 Task 的能力; 同时实现了多个接口

ASC 可以挂在在 `Owner Actor` 上, 初始化的时候会寻找 `OwnerActor` 上的 `AttributeSet` 进行注册

ASC 是运行的核心, 很多函数其实都是通过 ASC 转发

技能互相作用: 其实就是一个 Actor 上的 ASC 作用到另一个 Actor 上的 ASC

### 总结

![](Images/028.png)

## 简单使用

[GAS插件介绍（入门篇）](https://www.bilibili.com/video/BV1X5411V7jh/)

全称 `Gameplay Ability System` 简称 `GAS`， 是一个健壮的，高度可扩展的 `gameplay` 框架，通常用于构建 RPG、Moba 等游戏的完整战斗逻辑框架

通过 `GAS` ，可以快速地制作游戏中的 **主动**/**被动**技能、各种**效果buff**、**计算属性伤害**、**处理玩家各种战斗状态逻辑**

完整的技能框架需要: 可配置, 可编辑, 完整的制作流程, 联网…

`GAS` 提供的功能

- 实现了带有消耗和冷却功能的角色**技能**
- 处理**数值属性**(声明、魔法、攻击力、防御力)
- 应用**状态效果**(击飞、着火、眩晕)
- 应用**游戏标签**(GameplayTags)
- 生成**特效**和**音效**
- 完整的**网络复制**、**预测**功能

> 其他UE插件: **Able Ability System** 和 **Ascent Comba Framework**

### Ability System Component

`Ability System Component`(ASC) 是整个 GAS 的基础组件

ASC 本质上是一个 `UActorComponent`, 用于处理整个框架下的交互逻辑，包括使用**技能**(`GameplayAbility`)、 包含**属性**(`AttributeSet`)、处理各种**效果**(`GameplayEffect`)

所有需要应用 GAS 的对象 (Actor) 都必须拥有 GAS 组件

拥有 ASC 的 Actor 被称为 ASC 的 `OwnerActor`, ASC 实际作用的 Actor 叫做 `AvatarACtor`

ASC 可以被赋予某个角色 ASC, 可以被赋予 PlayerState (可以保存死亡角色的一些数据)

> 比如 Dota 角色死亡之后, 下次复活的时候大招CD仍然需要记录, 不能说死了一次大招CD没了

> 一些Demo项目 ASC 作用的永远都是 Character, 这样避免绕来绕去, 方便学习

### Gameplay Tags

`FGameplayTags` 是一种 层级标签, 比如 `Parent.Child.GrandChild`。通过 `GameplayTagManager` 进行注册

替代了原来的 `Bool` 或者 `Enum` 的结构, 可以在玩法设计上更高效的标记对象的行为或状态

![GamePlay Tags](./Images/001.png)

以前的常用方法是使用 枚举 和 Bool值 进行状态标记。每次开发的时候要先判断这状态属于哪一类等

Tags 是一种定义层级标签的方式, 有父子级关系。 所有 Tags 支持根据不同的状态、情况进行一个归类

![GamePlay Tags](./Images/002.png)

> 父级 **受限** 表命状态, 子集 **减速**、**击飞**、**眩晕** 表示具体的状态
> 比如一些技能可以移除 **受限** 状态, 那么只需要对父级标签 **受限** 进行处理, 其所有子集都会被影响

### Gameplay Ability

`Gameplay Ability` (GA) 标识了游戏中一个对象(Actor)可以做的**行为**或**技能**

能力(`Ability`) 可以是普通攻或者吟唱攻击, 可以是角色被击飞倒地, 还可以是使用某种道具, 交互某个物件, 甚至跳跃、飞行等角色行为也可以是 `Ablity`

`Ability` 可以被赋予对象或从对象的 `ASC` 中移除, 对象同时可以激活多个 `GameplayAbility`

> 基本的移动输入、UI交互行为则不能或不建议通过 GA 来实现

![GamePlay Ability](./Images/003.png)

上面就是一个 `GA` 的简单案例, 从 `Event ActivateAbility` 事件开始, `Montage` 播放结束时触发 `End Ability` , `montage` 播放是根据 `notify` 动画通知 添加 GE

右侧是这个 GA 的标签, 和这个 GA 会影响的其他系统的标签

![GamePlay Ability](./Images/004.png)

上面就是 GA 的一个简单调用流程

1. `TryActivateAbility` 尝试使用技能 开始进入 GA 释放流程
2. `CanActivateAbility` 判断能否使用技能
3. `ActivateAbility` 如果技能可以释放则触发该事件 进入 GA 内部的流程
4. 技能前摇
5. `CommitAbility` 确认技能释放, 开始消耗蓝条、进入CD等 (比如一些技能有释放前摇, 在完全释放出去之前都不进入CD)
   - `CommitAbility` 有三种: `CommitAbility、` `CommitAbilityCooldown、CommitAbilityCost`, 分别表示 同时消耗蓝和进入CD、只消耗蓝、只进入CD
   - 区分三种是因为可能存在这么一种技能: 释放时立刻消耗蓝, 在完成吟唱之后再进入CD
   - `CommitAbility` 本质上就是执行 AE, 所以手动执行对应的 AE 也可以完成功能
6. 技能释放中的一些操作
7. 技能后摇
8. `EndAbility` 当技能后摇的某个时间点被认定为技能结束时调用接口

### Gameplay Effect

`Gameplay Effect` 是 `Ability` 对自己或他人**产生影响**的途径

GE 通常可以被理解为游戏中的 `buff` (比如增益/减少效果、修改属性)

但是 GAS 中的 GE 更加广义, 释放技能时候的**伤害结算**, 施加特殊效果的**控制**、**霸体**效果 (修改 `GameplayTag`) 都是通过 GE 来实现的

> 当目标有 **霸体** 的 Tag 时, 移除负面 Tag 的 GE。通过这样可以实现目标效果

GE 相当于一个可配置的**数据表**, 不可以添加逻辑。 开发者创建一个 `UGameplayEffect` 的派生蓝图, 就可以根据需求制作想要的效果

![GamePlay Effect](./Images/005.png)

> 得自己尝试各个参数的意义… 翻译不一定正确 用了才知道

![GamePlay Effect](./Images/014.png)

当你的 GE 是持续一段时间 或者 无限时间的时候, `Period` 就表示周期性激活

- `Period` 表示时间间隔, 也就是 `interval`
- `Execute Periodic Effect on Application` 表示是否是一激活就执行
  - 先以 TS 的 `interval` 举例, TS 的 `interval` 在第0秒的时候并不会执行函数体, 而是第一个周期到了之后才会执行
  - 类似的, 这里表示 GE 激活的时候立刻执行扣血逻辑, 还是等待 interval 第一个周期到了再执行扣血逻辑

![GamePlay Effect](./Images/015.png)

GE 中的 Stacking 表示层, 比如: Dota 中蝙蝠可以给对方叠5层油。也就是一个 buff 可以叠加多层

![GamePlay Effect](./Images/016.png)

GE 也可以赋予能力

### Attribute Set

`AttributeSet` 负责定义和持有属性, 并且管理属性的变化, 包括网络同步

什么是属性: 生命值、魔法值、攻击力、防御力等

需要在 Actor 中添加为成员变量, 并注册到 ASC (C++)

一个 ASC 可以拥有一个或多个(不同)的 `AttributeSet`, 因此可以角色共享一个很大的 `AttributeSet，` 也可以每个角色按需添加 `AttributeSet`

可以在属性变化前(`PreAttributeChange`) 后(`PoasGameplayEffectExecute`) 处理相关逻辑, 可以通过委托的方式绑定属性变化

![GamePlay AttributeSet](./Images/006.png)

上图中的代码定义了一个名为 `Damage` 的属性,

`ATTRIBUTE_ACCESSORS(UFightAttributeSet, Damage)` 是一个宏定义，它创建了用于访问属性的一些便捷函数。这些函数包括获取属性的当前值、设置属性的当前值以及获取属性的最大值等

`OnRep_Damage` 是一个函数声明，它是属性 `Damage` 的变动时触发的属性修改通知函数。当 `Damage` 属性发生变化时，该函数会被调用。在该函数中，可以编写处理 `Damage` 变动的逻辑

上图下面的重写函数是 GAS 提供的回调函数, 用于属性或效果执行过程中进行自定义逻辑的处理

- `PreGameplayEffectExecute`: 在应用 `GameplayEffect` 之前被调用，允许自定义逻辑并决定是否执行效果。当返回值为 false 时，将取消效果的执行
- `PostGameplayEffectExecute`: 在应用 `GameplayEffect` 之后被调用，允许自定义逻辑处理效果的执行结果
- `PreAttributeChange`: 在属性值变化之前被调用，允许自定义逻辑并修改属性的新值
- `PostAttributeChange`: 在属性值变化之后被调用，允许自定义逻辑处理属性值的变化结果，并提供旧值和新值的信息
- `PreAttributeBaseChange`: 在属性基础值变化之前被调用，允许自定义逻辑并修改属性的新基础值
- `PostAttributeBaseChange`: 在属性基础值变化之后被调用，允许自定义逻辑处理属性基础值的变化结果，并提供旧值和新值的信息
- `OnAttributeAggregatorCreated`: 在创建属性聚合器（Aggregator）时被调用，允许自定义逻辑处理属性聚合器的创建过程

那么什么时候需要拆分多个 `AttributeSet` 呢?

拿英雄联盟举例，有些英雄没有蓝、有些英雄靠怒气、有些英雄靠能量.... 如果将这些属性全部放在一个 `AttributeSet` 中就显得臃肿也不好管理, 于是将通用部分拆出来

### GAS的使用流程

![使用GA](./Images/011.png)

点击攻击键攻击目标角色

1. 使用 `GA_Attack_Attr`

![使用GA](./Images/007.png)

使用 `Montage` 播放动画, 当 `Anim Notify` 触发的时候做射箭检测

给被攻击到的 `Actor` 的 `ASC` 添加 `GE_Hit` 和 `GE_Damage` 标签

除了运行逻辑, 还需要关注右边的 `Class Defaults` 

在 `Tags` 分类中

- `Ability Tags` 表示这个能力的标签
- `Calcel Abilities With Tag` 表示当此 GA 被激活时, 停止属于这个 Tag 的其他 GA 的运行 
- `Block Abilityies With Tag` 表示与此 GA 被激活时, 阻止属于这个 Tag 的其他 GA 被激活
- `Activation Blocked Tags` 表示当 Actor 存在这个 Tag 的时候, 这个 GA 不能被激活

这里将 `Ability Tag` 和 `Block Abilities With Tag` 设置成相同的 Tag, 可以保证玩家重复点击攻击键的时候角色不会出现鬼畜的情况

因为第一次攻击会给 Player 添加`Ability.Positive` 标签, 第二个点击攻击键时如果第一次攻击没有结束, 那么对应的标签也不会被删除, 那么第二次攻击的 GA 就不会激活

在 `Costs` 分类中 

- `Cost Gameplay Effet Class` 就是这个技能被 `Commit` 的时候的消耗(扣血、扣蓝等)

这里, 当 `GA_Attack` 运行到 `CommitAbility` 的时候, 会激活 `GE_Cost`

2. 使用 `GE_Hit` 

![使用GE](./Images/008.png)

- `Gameplay Effect` 中的 `Duration Policy`

该属性具有三种选项: `Instant`、`Infinite`、`HasDuration`, 分别表示 立即生效、永久存在、一定时间内存在

一般来说 `Instant` 就是单纯做伤害计算用的, 不会去做tag处理
一般来说 `Infinite` 做常驻技能, 比如被动 

![使用GE](./Images/009.png)

这里给被打的对象添加了一个 `Ability.Hit` 的标签

这个GE会持续 0.1s 的时间, 是因为我们觉得角色被打的僵直动作有 0.1s 

3. 触发 `GA_BeAttack`

![触发GA](./Images/010.png)

当被打的 `Actor` 添加了 `Ability.Hit` 标签, 那么这个 `GA_BeAttack` 就会被触发

被打的时候就播放一个 `Montage`,

这里被打也是一种能力(某些Boss就是没有被打的能力, 不会出现僵直)

注意这个 `GA_BeAttack` 的 `Triggers` 属性部分, 当 `Ability.Hit` 标签被添加的时候触发

这里 `Trigger Source` 有三种选项

- `Gameplay Event`: 使用 `SendGameplayEvent` 等节点调用, 触发指定 Tag 激活
- `Owned Tag Added`: 当指定 Tag 被添加到 Actor 上时被激活
- `Owned Tag Present`:  // TODO

4. 触发攻击消耗 `GE_Cost`, 配置在 `GA_Attack` 的 Cost 属性中

![触发Cost](./Images/012.png)

这里可以发现这个 GE 的 `Duration Policy` 是 `Instant` 的, 也就是立即执行的

它配置了一个 `Modifiers`, 这个 `Modifier` 的操作是对 `SampleAttributSet` 的 `Physical` 属性执行 `Add` 操作, 修改的数值是 `-5`, 说白了就是将角色的蓝量扣除5

5. 攻击触发扣血 `GE_Damage`

![触发GE_Damage](./Images/013.png)

同理对角色的血量进行扣除

### FGameplayAttributeData

我们发现 `AttributeSet` 中属性的值并不是float、int等普通数据类型, 而是 `FGameplayAttributeData` 这种类型

```c++
USTRUCT(BlueprintType)
struct GAMEPLAYABILITIES_API FGameplayAttributeData
{
	GENERATED_BODY()
    // ...
	FGameplayAttributeData(float DefaultValue)
		: BaseValue(DefaultValue)
		, CurrentValue(DefaultValue)
	{}

    // ...
}
```

可以看到 `FGameplayAttributeData` 有俩个值: `BaseValue` 和 `CurrentValue`

这么定义的原因是方便数据回滚, 比如Player得到一个buff可以增加攻击力10% 持续5s, 通过区分`BaseValue` 和 `CurrentValue`可以方便计算移除buff后的数值

