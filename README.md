# CinnamonCraft

Open source Minecraft beta 1.7.3 client written in C + OpenGL.

## Custom Packets

Not actually implemented yet, but planned!

- **modify item entry**: Modifies the item with the given ID (creating a new item if no such item exists). I'll figure out the specifics later, but it'll involve setting its name, metadata, block atlas index, etc.
- **modify item atlas entry**: Modifies the 16x16 texture at the specified position in the item atlas.
- **modify block entry**: Modifies the block with the given ID (creating a new block if no such block exists).
- **modify block atlas entry**: Modifies the 16x16 texture at the specified position in the block atlas.

These packets allow for server-side mods to automatically modify the client (without the client needing to manually mod their client).

## References

I like the medieval fantasy vibe of Minecraft, so I wanna lean into that. I also like the old console-edition crafting menu (couch gaming omg)

![](https://pbs.twimg.com/media/HFsXpGGXsAAr-Cl?format=jpg&name=small)

![](https://i.redd.it/4orvszi8284c1.png)

![](https://i.redd.it/e6biyog703ne1.png)

![](https://pbs.twimg.com/media/EkfTl0FVMAAaQ49.png)