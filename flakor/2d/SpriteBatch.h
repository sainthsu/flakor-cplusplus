/****************************************************************************
Copyright (c) 2013-2014 Flakor.org

http://www.flakor.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#ifndef _FK_SPRITE_BATCH_H_
#define _FK_SPRITE_BATCH_H_

#include <vector>

#include "2d/Sprite.h"


FLAKOR_NS_BEGIN

/**
 * @addtogroup _2d
 * @{
 */

class TextureAtlas;

/** SpriteBatch is like a batch node: if it contains children, it will draw them in 1 single OpenGL call
 * (often known as "batch draw").
 *
 * A SpriteBatch can reference one and only one texture (one image file, one texture atlas).
 * Only the Sprites that are contained in that texture can be added to the SpriteBatch.
 * All Sprites added to a SpriteBatch are drawn in one OpenGL ES draw call.
 * If the Sprites are not added to a SpriteBatch then an OpenGL ES draw call will be needed for each one, which is less efficient.
 *
 *
 * Limitations:
 *  - The only object that is accepted as child (or grandchild, grand-grandchild, etc...) is Sprite or any subclass of Sprite. eg: particles, labels and layer can't be added to a SpriteBatch.
 *  - Either all its children are Aliased or Antialiased. It can't be a mix. This is because "alias" is a property of the texture, and all the sprites share the same texture.
 *
 * @since v0.7.1
 */
class FK_DLL SpriteBatch : public Entity, public ITexture
{
    static const int DEFAULT_CAPACITY = 29;

public:
    /** Creates a SpriteBatch with a texture2d and capacity of children.
     * The capacity will be increased in 33% in runtime if it runs out of space.
     *
     * @param tex A texture2d.
     * @param capacity The capacity of children.
     * @return Return an autorelease object.
     */
    static SpriteBatch* createWithTexture(Texture2D* tex, ssize_t capacity = DEFAULT_CAPACITY);

    /** Creates a SpriteBatch with a file image (.png, .jpeg, .pvr, etc) and capacity of children.
     * The capacity will be increased in 33% in runtime if it runs out of space.
     * The file will be loaded using the TextureMgr.
     *
     * @param fileImage A file image (.png, .jpeg, .pvr, etc).
     * @param capacity The capacity of children.
     * @return Return an autorelease object.
     */
    static SpriteBatch* create(const std::string& fileImage, ssize_t capacity = DEFAULT_CAPACITY);


    /** Returns the TextureAtlas object. 
     *
     * @return The TextureAtlas object.
     */
    inline TextureAtlas* getTextureAtlas() { return _textureAtlas; }

    /** Sets the TextureAtlas object. 
     *
     * @param textureAtlas The TextureAtlas object.
     */
    inline void setTextureAtlas(TextureAtlas* textureAtlas)
    { 
        if (textureAtlas != _textureAtlas)
        {
            FK_SAFE_RETAIN(textureAtlas);
            FK_SAFE_RELEASE(_textureAtlas);
            _textureAtlas = textureAtlas;
        }
    }

    /** Returns an array with the descendants (children, gran children, etc.).
     * This is specific to BatchNode. In order to use the children, use getChildren() instead.
     * 
     * @return An array with the descendants (children, gran children, etc.).
     */
    inline const std::vector<Sprite*>& getDescendants() const { return _descendants; }

    /** Increase the Atlas Capacity. */
    void increaseAtlasCapacity();

    /** Removes a child given a certain index. It will also cleanup the running actions depending on the cleanup parameter.
     *
     * @param index A certain index.
     * @param doCleanup Whether or not to cleanup the running actions.
     * @warning Removing a child from a SpriteBatch is very slow.
     */
    void removeChildAtIndex(ssize_t index, bool doCleanup);
    
    /** Append the child. 
     *
     * @param sprite A Sprite.
     */
    void appendChild(Sprite* sprite);
    
    /** Remove a sprite from Atlas. 
     *
     * @param sprite A Sprite.
     */
    void removeSpriteFromAtlas(Sprite *sprite);
    
    /** Rebuild index with a sprite all child. 
     *
     * @param parent The parent sprite.
     * @param index The child index.
     * @return Index.
     */
    ssize_t rebuildIndexInOrder(Sprite *parent, ssize_t index);
    
    /** Get the Max image block index,in all child.
     *
     * @param sprite The parent sprite.
     * @return Index.
     */
    ssize_t highestAtlasIndexInChild(Sprite *sprite);
    
    /** Get the Min image block index,in all child. 
     *
     * @param sprite The parent sprite.
     * @return Index.
     */
    ssize_t lowestAtlasIndexInChild(Sprite *sprite);
    
    /** Get the nearest index from the sprite in z.
     *
     * @param sprite The parent sprite.
     * @param z Z order for drawing priority.
     * @return Index.
     */
    ssize_t atlasIndexForChild(Sprite *sprite, int z);
    /* Sprites use this to start sortChildren, don't call this manually. */
    void reorderBatch(bool reorder);

    //
    // Overrides
    //
    // TextureProtocol
    virtual Texture2D* getTexture() const override;
    virtual void setTexture(Texture2D *texture) override;
    /**
    *@code
    * When this function bound into js or lua,the parameter will be changed.
    * In js: var setBlendFunc(var src, var dst).
    * @endcode
    * @lua NA 
    */
    virtual void setBlendFunc(const BlendFunc &blendFunc) override;
    /**
    * @js NA
    * @lua NA
    */
    virtual const BlendFunc& getBlendFunc() const override;

    virtual void visit(Renderer *renderer, const Mat4 &parentTransform, uint32_t parentFlags) override;
    
    using Entity::addChild;
    virtual void addChild(Entity * child, int zOrder, int tag) override;
    virtual void reorderChild(Entity *child, int zOrder) override;
        
    virtual void removeChild(Entity *child, bool cleanup) override;
    virtual void removeAllChildren(bool cleanup) override;
    virtual void sortAllChildren() override;
    virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
    virtual std::string getDescription() const override;

    /** Inserts a quad at a certain index into the texture atlas. The Sprite won't be added into the children array.
     * This method should be called only when you are dealing with very big AtlasSrite and when most of the Sprite won't be updated.
     * For example: a tile map (TMXMap) or a label with lots of characters (LabelBMFont).
     */
    void insertQuadFromSprite(Sprite *sprite, ssize_t index);
    /* This is the opposite of "addQuadFromSprite.
     * It add the sprite to the children and descendants array, but it doesn't update add it to the texture atlas
     */
    SpriteBatch * addSpriteWithoutQuad(Sprite *child, int z, int aTag);
    
FK_CONSTRUCTOR_ACCESS:
    /**
     * @js ctor
     */
    SpriteBatch();
    /**
     * @js NA
     * @lua NA
     */
    virtual ~SpriteBatch();
    
    /** initializes a SpriteBatch with a texture2d and capacity of children.
     The capacity will be increased in 33% in runtime if it runs out of space.
     */
    bool initWithTexture(Texture2D *tex, ssize_t capacity = DEFAULT_CAPACITY);
    /** initializes a SpriteBatch with a file image (.png, .jpeg, .pvr, etc) and a capacity of children.
     The capacity will be increased in 33% in runtime if it runs out of space.
     The file will be loaded using the TextureMgr.
     * @js init
     * @lua init
     */
    bool initWithFile(const std::string& fileImage, ssize_t capacity = DEFAULT_CAPACITY);
    bool init() override;
    
protected:
    /** Updates a quad at a certain index into the texture atlas. The Sprite won't be added into the children array.
     This method should be called only when you are dealing with very big AtlasSrite and when most of the Sprite won't be updated.
     For example: a tile map (TMXMap) or a label with lots of characters (LabelBMFont)
     */
    void updateQuadFromSprite(Sprite *sprite, ssize_t index);   

    void updateAtlasIndex(Sprite* sprite, ssize_t* curIndex);
    void swap(ssize_t oldIndex, ssize_t newIndex);
    void updateBlendFunc();

    TextureAtlas *_textureAtlas;
    BlendFunc _blendFunc;
    BatchCommand _batchCommand;     // render command

    // all descendants: children, grand children, etc...
    // There is not need to retain/release these objects, since they are already retained by _children
    // So, using std::vector<Sprite*> is slightly faster than using cocos2d::Array for this particular case
    std::vector<Sprite*> _descendants;
};

// end of sprite group
/** @} */

FLAKOR_NS_END

#endif
