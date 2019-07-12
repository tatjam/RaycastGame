#pragma once
#include "SpriteEntity.h"
#include "InventorySlot.h"

class InventoryEntity;


// Items can exist either inside inventories, or as an sprite on the world
// so we inherit SpriteEntity, but it may not actually have a sprite all the time
// Commands:
//
// Generic Calls:
// 	- setItemFrame (SERVER->CLIENT)
//		-> int frame
//  - contextualMenuAction (SERVER<->CLIENT)
//		-> string action
//	- useOnItem (SERVER<->CLIENT)
//		-> uint32_t uid
//	- useOnSprite (SERVER<->CLIENT)
//		-> uint32_t uid
//		-> float texX
//		-> float texY
//	- useOnTile (SERVER<->CLIENT)
//		-> int x
//		-> int y
//		-> float texX
//		-> float texY
//		-> int side
//

class ItemEntity : public SpriteEntity
{
private:

	InventoryEntity* inventory;
	bool inWorld;
	bool inSpecial;


	sf::Vector2i inventoryPos;

	SpecialSlot specialPos;

	// We don't need a texture for the world 
	// as it's handled by the SpriteEntity
	sf::Texture textureInInventory;
	sf::Texture textureInHand;

	// Frame used when rendering to hand or inventory
	// Used for animations or states
	size_t itemFrame;

	// Used for the default animation (draw thing)
	sf::Vector2f handPos;
	bool wasInSpecial;
	float handTimer;

public:

	// Should not be called while in the world
	InventoryEntity* getInventory();
	void setInventory(InventoryEntity* inv);
	
	bool isInInventory() { return !inWorld; }
	bool isInInventorySlot() { return !inWorld && !inSpecial; }
	bool isInWorld() { return inWorld; }
	bool isInSpecial() { return inSpecial; }
	void setInWorld(sf::Vector2f pos);
	void setInInventory(sf::Vector2i pos);
	void setInSpecialSlot(SpecialSlot special);
	SpecialSlot getSpecialSlot()
	{
		if (inSpecial)
		{
			return specialPos;
		}
		else
		{
			return SpecialSlot::NONE;
		}
	}

	sf::Texture& getTextureInInventory() { return textureInInventory; }
	sf::Texture& getTextureInHand() { return textureInHand; }

	void setImageInInventory(sf::Image* img) { textureInInventory.loadFromImage(*img); }
	void setImageInHand(sf::Image* img) { textureInHand.loadFromImage(*img); }

	virtual void onGenericCall(std::string cmd, json args, ENetPeer* peer) override;

	// Should only be called on the server IF using network
	// for client-side animations it's safe to call it in the client
	void setItemFrame(size_t frame, bool sendNetwork = false);
	size_t getItemFrame() { return itemFrame; }

	// Should not be called while in the world
	sf::Vector2i getInventoryPos();

	// You should not use this to set an item position
	// it's meant for internal use by the InventoryEntity
	// as using this will ignore event calls in the inventory
	// and safety checks
	void setInventoryPos(sf::Vector2i pos);

	// Should be overwritten by certain items
	// Can be called anytime
	virtual sf::Vector2i getInventorySize();

	virtual sf::Vector2f getHandPos();
	virtual sf::Vector2f getHandScale();
	virtual sf::Color getHandTint();

	// By default items cannot be worn on special slots,
	// but overriding may change this behaviour
	virtual bool canGoInSlot(InventorySlot slot)
	{
		return slot == InventorySlot::NORMAL;
	}

	// Can the item be held in one hand, or does it require both?
	virtual bool isOneHanded()
	{
		return true;
	}

	bool isHeldInHands()
	{
		return getSpecialSlot() == L_HAND || getSpecialSlot() == R_HAND || getSpecialSlot() == B_HAND;
	}

	virtual json serialize() override;
	virtual void deserialize(json j) override;

	virtual void update(float dt) override;

	

	// Called on both the server and client, but the heavyweight
	// should be done on the server
	// They are automatically called from the helper functions
	
	// Send goes to server (always)
	// Also calls doContextualMenuAction on the client (always)
	void sendContextualMenuAction(std::string action);
	// On the server this aditionally calls receive on all clients
	void receiveContextualMenuAction(std::string action, ENetPeer* from);

	// Called when you right-click an item that's in a hand slot
	// to know which options to display on the contextual menu
	// Any entry ending with "#" will be disabled
	virtual std::vector<std::string> getContextualMenuEntries() {return std::vector<std::string>();}

	// Called when you left-click a contextual menu entry
	virtual void doContextualMenuAction(std::string action) {}

	// Called when the user clicks use in the contextual menu
	// and clicks another item on the inventory. Note that dropped items
	// go into useOnSprite
	virtual void useOnItem(ItemEntity* target) {}

	// Called when the user either uses the appropiate button in FPS mode
	// while aiming at an sprite or when the user clicks use in the contextual
	// menu and clicks an sprite on the screen
	virtual void useOnSprite(SpriteEntity* target, sf::Vector2f texCoord) {}

	// Called when the user either uses the appropiate button in FPS mode
	// while aiming at a tile or when the user clicks 
	virtual void useOnTile(sf::Vector2i pos, Side side, sf::Vector2f texCoord) {}

	ItemEntity(ProgramType* prog, uint32_t uid) : SpriteEntity(prog, uid), Entity(prog, uid)
	{
		itemFrame = 0;
		wasInSpecial = false;
		handTimer = 0.0f;
	}

	~ItemEntity();
};

