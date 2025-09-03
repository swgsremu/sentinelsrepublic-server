#ifndef SRSTRUCTUREOBJECT_H
#define SRSTRUCTUREOBJECT_H

#include <server/zone/objects/intangible/ControlDevice.h>
#include <server/zone/objects/tangible/sign/SignObject.h>
#include "engine/engine.h"
#include "system/thread/ReadWriteLock.h"
#include "system/thread/ReadLocker.h"

// Forward declaration
namespace server {
    namespace zone {
        namespace objects {
            namespace tangible {
                namespace sign {
                    class SignObject;
                }
            }
        }
    }
}

/**
 * @brief Class representing a structure object in the SR system.
 */
class SRStructureObject : public Object
{
    mutable ReadWriteLock lock; /**< Lock for thread-safe access. */
    ControlDevice* controlDevice; /**< Pointer to the control device associated with this structure. */
    
    // Item position information
    struct ItemPositionData {
        uint64 objectID;
        float posX;
        float posY;
        float posZ;
        float directionAngle; // Store direction as a simple angle instead of Quaternion
    };
    
    // Transient storage of packed items per cell number with position data
    std::unordered_map<int, std::vector<ItemPositionData>> packedCellItems;
    
    // Sign information storage
    String signTemplatePath; // Store the template path of the sign

private:
    /**
     * @brief Writes the object members to the output stream.
     *
     * @param stream The output stream to write to.
     * @return int Status code indicating success or failure.
     */
    int writeObjectMembers(ObjectOutputStream* stream);

    /**
     * @brief Reads an object member from the input stream.
     *
     * @param stream The input stream to read from.
     * @param name The name of the member to read.
     * @return bool True if the member was successfully read, false otherwise.
     */
    bool readObjectMember(ObjectInputStream* stream, const String& name);

protected:
public:
    /**
     * @brief Default constructor for SRStructureObject.
     */
    SRStructureObject();

    /**
     * @brief Destructor for SRStructureObject.
     */
    ~SRStructureObject() override;

    /**
     * @brief Copy constructor for SRStructureObject.
     *
     * @param spl The SRStructureObject to copy from.
     */
    SRStructureObject(const SRStructureObject& spl);

    /**
     * @brief Assignment operator for SRStructureObject.
     *
     * @param list The SRStructureObject to assign from.
     * @return SRStructureObject& Reference to the assigned object.
     */
    SRStructureObject& operator=(const SRStructureObject& list);

    /**
     * @brief Serializes the object to a binary stream.
     *
     * @param stream The output stream to write to.
     * @return bool True if the object was successfully serialized, false otherwise.
     */
    bool toBinaryStream(ObjectOutputStream* stream) override;

    /**
     * @brief Deserializes the object from a binary stream.
     *
     * @param stream The input stream to read from.
     * @return bool True if the object was successfully deserialized, false otherwise.
     */
    bool parseFromBinaryStream(ObjectInputStream* stream) override;

    /**
     * @brief Converts the object to JSON format.
     *
     * @param j The JSON object to write to.
     * @param p The SRStructureObject to convert.
     */
    friend void to_json(nlohmann::json& j, const SRStructureObject& p);

    /**
     * @brief Gets the packup message for the structure.
     *
     * @return String The packup message.
     */
    static String getPackupMessage();

    /**
     * @brief Sets the control device for the structure.
     *
     * @param device Pointer to the control device.
     */
    void setControlDevice(ControlDevice* device);

    /**
     * @brief Gets the control device for the structure.
     *
     * @return ControlDevice* Pointer to the control device.
     */
    ControlDevice* getControlDevice() const;

    /**
     * @brief Checks if the structure is packed up.
     *
     * @return bool True if the structure is packed up, false otherwise.
     */
    bool isPackedUp() const;

    /**
     * @brief Unloads the structure from the zone.
     *
     * @param sendSelfDestroy Flag indicating whether to send a self-destroy message.
     * @return bool True if the structure was successfully unloaded, false otherwise.
     */
    static bool unloadFromZone(bool sendSelfDestroy);

    // SR helpers: collect and restore items during pack/unpack (not persisted)
    void clearPackedItems();
    void collectItems(class BuildingObject* building);
    void restoreItems(class BuildingObject* building, class ZoneServer* zoneServer);
    
    // Sign helpers
    void saveSignInfo(server::zone::objects::tangible::sign::SignObject* sign);
    String getSignTemplatePath() const { 
        ReadLocker rlocker(&lock); 
        return signTemplatePath; 
    }
};

#endif // SRSTRUCTUREOBJECT_H
