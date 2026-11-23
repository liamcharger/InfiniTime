#include "components/ble/AppleNotificationCenterClient.h"
#include "components/ble/NotificationManager.h"
#include "systemtask/SystemTask.h"
#include "displayapp/screens/Symbols.h"
#include <algorithm>
#include <cstring>

using namespace Pinetime::Controllers;
namespace Symbols = Pinetime::Applications::Screens::Symbols;

int OnDiscoveryEventCallback(uint16_t conn_handle, const struct ble_gatt_error* error, const struct ble_gatt_svc* service, void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnDiscoveryEvent(conn_handle, error, service);
}

int OnANCSCharacteristicDiscoveredCallback(uint16_t conn_handle,
                                           const struct ble_gatt_error* error,
                                           const struct ble_gatt_chr* chr,
                                           void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnCharacteristicsDiscoveryEvent(conn_handle, error, chr);
}

int OnANCSDescriptorDiscoveryEventCallback(uint16_t conn_handle,
                                           const struct ble_gatt_error* error,
                                           uint16_t chr_val_handle,
                                           const struct ble_gatt_dsc* dsc,
                                           void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnDescriptorDiscoveryEventCallback(conn_handle, error, chr_val_handle, dsc);
}

int NewAlertSubcribeCallback(uint16_t conn_handle, const struct ble_gatt_error* error, struct ble_gatt_attr* attr, void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnNewAlertSubcribe(conn_handle, error, attr);
}

int OnControlPointWriteCallback(uint16_t conn_handle, const struct ble_gatt_error* error, struct ble_gatt_attr* attr, void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnControlPointWrite(conn_handle, error, attr);
}

AppleNotificationCenterClient::AppleNotificationCenterClient(Pinetime::System::SystemTask& systemTask,
                                                             Pinetime::Controllers::NotificationManager& notificationManager)
  : systemTask {systemTask}, notificationManager {notificationManager} {
}

bool AppleNotificationCenterClient::OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error* error, const ble_gatt_svc* service) {
  if (service == nullptr && error->status == BLE_HS_EDONE) {
    if (isDiscovered) {
      NRF_LOG_INFO("ANCS Discovery found, starting characteristics discovery");
      ble_gattc_disc_all_chrs(connectionHandle, ancsStartHandle, ancsEndHandle, OnANCSCharacteristicDiscoveredCallback, this);
    } else {
      NRF_LOG_INFO("ANCS not found");
      MaybeFinishDiscovery(connectionHandle);
    }
    return true;
  }

  if (service != nullptr && ble_uuid_cmp(&ancsUuid.u, &service->uuid.u) == 0) {
    NRF_LOG_INFO("ANCS discovered : 0x%x - 0x%x", service->start_handle, service->end_handle);

    ancsStartHandle = service->start_handle;
    ancsEndHandle = service->end_handle;
    isDiscovered = true;
  }
  return false;
}

int AppleNotificationCenterClient::OnCharacteristicsDiscoveryEvent(uint16_t connectionHandle,
                                                                   const ble_gatt_error* error,
                                                                   const ble_gatt_chr* characteristic) {
  if (error->status != 0 && error->status != BLE_HS_EDONE) {
    NRF_LOG_INFO("ANCS Characteristic discovery ERROR");
    MaybeFinishDiscovery(connectionHandle);
    return 0;
  }

  if (characteristic == nullptr && error->status == BLE_HS_EDONE) {
    NRF_LOG_INFO("ANCS Characteristic discovery complete");
    if (isCharacteristicDiscovered) {
      ble_gattc_disc_all_dscs(connectionHandle, notificationSourceHandle, ancsEndHandle, OnANCSDescriptorDiscoveryEventCallback, this);
    }
    if (isDataCharacteristicDiscovered) {
      ble_gattc_disc_all_dscs(connectionHandle, dataSourceHandle, ancsEndHandle, OnANCSDescriptorDiscoveryEventCallback, this);
    }
    if (isCharacteristicDiscovered == isControlCharacteristicDiscovered && isCharacteristicDiscovered == isDataCharacteristicDiscovered) {
      MaybeFinishDiscovery(connectionHandle);
    }
  } else if (characteristic != nullptr) {
    if (ble_uuid_cmp(&notificationSourceChar.u, &characteristic->uuid.u) == 0) {
      NRF_LOG_INFO("ANCS Characteristic discovered: Notification Source");
      notificationSourceHandle = characteristic->val_handle;
      isCharacteristicDiscovered = true;
    } else if (ble_uuid_cmp(&controlPointChar.u, &characteristic->uuid.u) == 0) {
      NRF_LOG_INFO("ANCS Characteristic discovered: Control Point");
      controlPointHandle = characteristic->val_handle;
      isControlCharacteristicDiscovered = true;
    } else if (ble_uuid_cmp(&dataSourceChar.u, &characteristic->uuid.u) == 0) {
      NRF_LOG_INFO("ANCS Characteristic discovered: Data Source");
      dataSourceHandle = characteristic->val_handle;
      isDataCharacteristicDiscovered = true;
    }
  }
  return 0;
}

int AppleNotificationCenterClient::OnDescriptorDiscoveryEventCallback(uint16_t connectionHandle,
                                                                      const ble_gatt_error* error,
                                                                      uint16_t characteristicValueHandle,
                                                                      const ble_gatt_dsc* descriptor) {
  if (error->status == 0) {
    if (characteristicValueHandle == notificationSourceHandle && ble_uuid_cmp(&notificationSourceChar.u, &descriptor->uuid.u)) {
      if (notificationSourceDescriptorHandle == 0) {
        NRF_LOG_INFO("ANCS Descriptor discovered : %d", descriptor->handle);
        notificationSourceDescriptorHandle = descriptor->handle;
        isDescriptorFound = true;
        uint8_t value[2] {1, 0};
        ble_gattc_write_flat(connectionHandle, notificationSourceDescriptorHandle, value, sizeof(value), NewAlertSubcribeCallback, this);
        ble_gattc_write_flat(connectionHandle, ancsEndHandle, value, sizeof(value), NewAlertSubcribeCallback, this);
      }
    } else if (characteristicValueHandle == controlPointHandle && ble_uuid_cmp(&controlPointChar.u, &descriptor->uuid.u)) {
      if (controlPointDescriptorHandle == 0) {
        NRF_LOG_INFO("ANCS Descriptor discovered : %d", descriptor->handle);
        controlPointDescriptorHandle = descriptor->handle;
        isControlDescriptorFound = true;
      }
    } else if (characteristicValueHandle == dataSourceHandle && ble_uuid_cmp(&dataSourceChar.u, &descriptor->uuid.u)) {
      if (dataSourceDescriptorHandle == 0) {
        NRF_LOG_INFO("ANCS Descriptor discovered : %d", descriptor->handle);
        dataSourceDescriptorHandle = descriptor->handle;
        isDataDescriptorFound = true;
        uint8_t value[2] {1, 0};
        ble_gattc_write_flat(connectionHandle, dataSourceDescriptorHandle, value, sizeof(value), NewAlertSubcribeCallback, this);
      }
    }
  } else {
    if (error->status != BLE_HS_EDONE) {
      char errorStr[55];
      snprintf(errorStr, sizeof(errorStr), "ANCS Descriptor discovery ERROR: %d", error->status);
      NRF_LOG_INFO(errorStr);
    }
    if (isDescriptorFound == isDataDescriptorFound)
      MaybeFinishDiscovery(connectionHandle);
  }
  return 0;
}

int AppleNotificationCenterClient::OnNewAlertSubcribe(uint16_t connectionHandle,
                                                      const ble_gatt_error* error,
                                                      ble_gatt_attr* /*attribute*/) {
  if (error->status == 0) {
    NRF_LOG_INFO("ANCS New alert subscribe OK");

    // Mark subscriptions complete only after both CCCDs are known
    if (notificationSourceDescriptorHandle != 0 && dataSourceDescriptorHandle != 0) {
      subscriptionsDone = true;
    }
  } else {
    NRF_LOG_INFO("ANCS New alert subscribe ERROR");
  }
  if (isDescriptorFound == isControlDescriptorFound && isDescriptorFound == isDataDescriptorFound)
    MaybeFinishDiscovery(connectionHandle);

  return 0;
}

int AppleNotificationCenterClient::OnControlPointWrite(uint16_t /*connectionHandle*/,
                                                       const ble_gatt_error* error,
                                                       ble_gatt_attr* /*attribute*/) {
  if (error->status == 0) {
    NRF_LOG_INFO("ANCS Control Point write OK");
  } else {
    char errorStr[55];
    snprintf(errorStr, sizeof(errorStr), "ANCS Control Point ERROR: %d", error->status);
    NRF_LOG_INFO(errorStr);
  }
  return 0;
}

void AppleNotificationCenterClient::MaybeFinishDiscovery(uint16_t connectionHandle) {
  if (isCharacteristicDiscovered && isControlCharacteristicDiscovered && isDataCharacteristicDiscovered && isDescriptorFound &&
      isControlDescriptorFound && isDataDescriptorFound && subscriptionsDone) {
    onServiceDiscovered(connectionHandle);
  }
}

// App list taken from Titus Kendzorra's InfiniBros fork (AppIdToEmoji in the lil-watch branches). His uses a
// std::unordered_map, which allocates on the heap. This uses a flat table in flash instead.
// Dropped from his list: Signal, Reddit, X, TikTok, Teams, Duolingo, ChatGPT, and his own banking, gym and birthday apps.
const char* AppleNotificationCenterClient::IconForAppId(const char* appId) {
  struct AppIcon {
    const char* appId;
    const char* icon;
  };

  static constexpr AppIcon appIcons[] = {
    {"net.whatsapp.WhatsApp", Symbols::whatsappLogo},
    {"com.burbn.instagram", Symbols::instagramLogo},
    {"com.hammerandchisel.discord", Symbols::discordLogo},
    {"com.google.ios.youtube", Symbols::youtubeLogo},
    {"com.toyopagroup.picaboo", Symbols::snapchatLogo},
    {"com.github.stormbreaker.prod", Symbols::githubLogo},
    {"com.apple.podcasts", Symbols::podcastsLogo},
    {"com.apple.MobileSMS", Symbols::messagesLogo},
    {"com.apple.reminders", Symbols::remindersLogo},
    {"com.apple.shortcuts", Symbols::shortcutsLogo},
    {"com.apple.Music", Symbols::musicLogo},
    {"com.apple.mobilephone", Symbols::phoneLogo},
    {"com.apple.Passbook", Symbols::walletLogo},
    {"com.apple.mobilemail", Symbols::mailLogo},
  };

  if (appId == nullptr || appId[0] == '\0') {
    return nullptr;
  }

  for (const auto& entry : appIcons) {
    if (std::strcmp(entry.appId, appId) == 0) {
      return entry.icon;
    }
  }

  // Any other Apple app gets the Apple logo
  if (std::strncmp(appId, "com.apple.", 10) == 0) {
    return Symbols::appleLogo;
  }

  return nullptr;
}

// Emoji list taken from Titus Kendzorra's InfiniBros fork (MapEmojiToSymbol in the lil-watch branches)
const char* AppleNotificationCenterClient::SymbolForEmoji(uint32_t codepoint) {
  struct EmojiSymbol {
    uint32_t codepoint;
    const char* symbol;
  };

  static constexpr EmojiSymbol emojiSymbols[] = {
    {0x1F44D, Symbols::thumbsUp},     {0x1F44E, Symbols::thumbsDown},  {0x1F642, Symbols::smile},         {0x1F62B, Symbols::tired},
    {0x1F62E, Symbols::openMouth},    {0x1F609, Symbols::wink},        {0x1F60A, Symbols::grin},          {0x1F622, Symbols::sad},
    {0x1F62D, Symbols::cry},          {0x1F644, Symbols::rollingEyes}, {0x1F636, Symbols::noMouth},       {0x1F610, Symbols::neutralFace},
    {0x1F606, Symbols::grinSquint},   {0x1F604, Symbols::grinSmile},   {0x1F600, Symbols::grinOpenMouth}, {0x1F618, Symbols::blowingKiss},
    {0x1F619, Symbols::kissGrin},     {0x1F617, Symbols::kiss},        {0x1F603, Symbols::grinWide},      {0x1F61C, Symbols::winkTongue},
    {0x1F61D, Symbols::squintTongue}, {0x1F61B, Symbols::grinTongue},  {0x1F602, Symbols::laugh},         {0x1F929, Symbols::grinStars},
    {0x1F923, Symbols::rofl},         {0x1F60D, Symbols::heartEyes},   {0x1F605, Symbols::sweatSmile},    {0x1F62C, Symbols::grimace},
    {0x1F641, Symbols::slightFrown},  {0x2639, Symbols::frown},        {0x1F633, Symbols::flushed},       {0x1F635, Symbols::deadEyes},
    {0x1F620, Symbols::angry},        {0x1F621, Symbols::angry},       {0x1F92C, Symbols::angry},         {0x2764, Symbols::heartEmoji},
    {0x1F64F, Symbols::prayingHands}, {0x1F91D, Symbols::handshake},   {0x270C, Symbols::peaceSign},      {0x1F595, Symbols::middleFinger},
    {0x1F4F7, Symbols::camera},       {0x1F4F9, Symbols::videoCam},    {0x1F3A4, Symbols::microphone},    {0x1F525, Symbols::flame},
  };

  for (const auto& entry : emojiSymbols) {
    if (entry.codepoint == codepoint) {
      return entry.symbol;
    }
  }
  return nullptr;
}

void AppleNotificationCenterClient::OnNotification(ble_gap_event* event) {
  if (event->notify_rx.attr_handle == notificationSourceHandle || event->notify_rx.attr_handle == notificationSourceDescriptorHandle) {
    NRF_LOG_INFO("ANCS Notification received");

    AncsNotification ancsNotif;

    os_mbuf_copydata(event->notify_rx.om, 0, 1, &ancsNotif.eventId);
    os_mbuf_copydata(event->notify_rx.om, 1, 1, &ancsNotif.eventFlags);
    os_mbuf_copydata(event->notify_rx.om, 2, 1, &ancsNotif.category);
    // Can be used to see how many grouped notifications are present
    // os_mbuf_copydata(event->notify_rx.om, 3, 1, &categoryCount);
    os_mbuf_copydata(event->notify_rx.om, 4, 4, &ancsNotif.uuid);

    // bool silent = (ancsNotif.eventFlags & static_cast<uint8_t>(EventFlags::Silent)) != 0;
    //  bool important = eventFlags & static_cast<uint8_t>(EventFlags::Important);
    // bool preExisting = (ancsNotif.eventFlags & static_cast<uint8_t>(EventFlags::PreExisting)) != 0;
    //  bool positiveAction = eventFlags & static_cast<uint8_t>(EventFlags::PositiveAction);
    //  bool negativeAction = eventFlags & static_cast<uint8_t>(EventFlags::NegativeAction);

    // If notification was removed, we remove it from the notifications map
    if (ancsNotif.eventId == static_cast<uint8_t>(EventIds::Removed) && notifications.contains(ancsNotif.uuid)) {
      notifications.erase(ancsNotif.uuid);
      NRF_LOG_INFO("ANCS Notification removed: %d", ancsNotif.uuid);
      return;
    }

    // If the notification is pre-existing, or if it is a silent notification, we do not add it to the list
    if (notifications.contains(ancsNotif.uuid) || (ancsNotif.eventFlags & static_cast<uint8_t>(EventFlags::Silent)) != 0 ||
        (ancsNotif.eventFlags & static_cast<uint8_t>(EventFlags::PreExisting)) != 0) {
      return;
    }

    // If new notification, add it to the notifications
    if (ancsNotif.eventId == static_cast<uint8_t>(EventIds::Added) &&
        (ancsNotif.eventFlags & static_cast<uint8_t>(EventFlags::Silent)) == 0) {
      notifications.insert({ancsNotif.uuid, ancsNotif});
    } else {
      // If the notification is not added, we ignore it
      NRF_LOG_INFO("ANCS Notification not added, ignoring: %d", ancsNotif.uuid);
      return;
    }

    // The 6 is from TotalNbNotifications in NotificationManager.h + 1
    while (notifications.size() > 100) {
      notifications.erase(notifications.begin());
    }

    // if (notifications.contains(ancsNotif.uuid)) {
    //   notifications[ancsNotif.uuid] = ancsNotif;
    // } else {
    //   notifications.insert({ancsNotif.uuid, ancsNotif});
    // }

    // Request ANCS more info
    // The +4 is for the "..." at the end of the string
    uint8_t titleSize = maxTitleSize + 4;
    uint8_t subTitleSize = maxSubtitleSize + 4;
    uint8_t messageSize = maxMessageSize + 4;
    BYTE request[15];
    request[0] = 0x00; // Command ID: Get Notification Attributes
    request[1] = static_cast<uint8_t>(ancsNotif.uuid & 0xFF);
    request[2] = static_cast<uint8_t>((ancsNotif.uuid >> 8) & 0xFF);
    request[3] = static_cast<uint8_t>((ancsNotif.uuid >> 16) & 0xFF);
    request[4] = static_cast<uint8_t>((ancsNotif.uuid >> 24) & 0xFF);
    request[5] = 0x01; // Attribute ID: Title
    // request[6] = 0x00;
    request[6] = static_cast<uint8_t>(titleSize & 0xFF);
    request[7] = static_cast<uint8_t>((titleSize >> 8) & 0xFF);
    request[8] = 0x02; // Attribute ID: Subtitle
    request[9] = static_cast<uint8_t>(subTitleSize & 0xFF);
    request[10] = static_cast<uint8_t>((subTitleSize >> 8) & 0xFF);
    request[11] = 0x03; // Attribute ID: Message
    request[12] = static_cast<uint8_t>(messageSize & 0xFF);
    request[13] = static_cast<uint8_t>((messageSize >> 8) & 0xFF);
    // Asking for the app identifier comes from Titus Kendzorra's InfiniBros fork. Unlike title, subtitle
    // and message, it takes no max length parameter.
    request[14] = 0x00; // Attribute ID: App Identifier

    ble_gattc_write_flat(event->notify_rx.conn_handle, controlPointHandle, request, sizeof(request), OnControlPointWriteCallback, this);
  } else if (event->notify_rx.attr_handle == dataSourceHandle || event->notify_rx.attr_handle == dataSourceDescriptorHandle) {
    uint16_t titleSize;
    uint16_t subTitleSize;
    uint16_t messageSize;
    uint32_t notificationUid;

    os_mbuf_copydata(event->notify_rx.om, 1, 4, &notificationUid);
    os_mbuf_copydata(event->notify_rx.om, 6, 2, &titleSize);
    os_mbuf_copydata(event->notify_rx.om, 8 + titleSize + 1, 2, &subTitleSize);
    os_mbuf_copydata(event->notify_rx.om, 8 + titleSize + 1 + 2 + subTitleSize + 1, 2, &messageSize);

    // The app identifier comes after the message. It can be missing, e.g. when the phone splits a long
    // response across several packets, and then the notification just has no logo.
    const uint16_t appIdSizeOffset = 8 + titleSize + 1 + 2 + subTitleSize + 1 + 2 + messageSize + 1;
    std::array<char, maxAppIdSize + 1> appId {};
    uint16_t appIdSize = 0;
    if (os_mbuf_copydata(event->notify_rx.om, appIdSizeOffset, 2, &appIdSize) == 0 && appIdSize <= maxAppIdSize) {
      if (os_mbuf_copydata(event->notify_rx.om, appIdSizeOffset + 2, appIdSize, appId.data()) != 0) {
        appId[0] = '\0';
      }
    }

    AncsNotification ancsNotif;
    ancsNotif.uuid = 0;

    // Check if the notification is in the session
    if (notifications.contains(notificationUid)) {
      if (notifications[notificationUid].isProcessed) {
        // If the notification is already processed, we ignore it
        NRF_LOG_INFO("Notification with UID %d already processed, ignoring", notificationUid);
        return;
      }
    } else {
      // If the notification is not in the session, we ignore it
      NRF_LOG_INFO("Notification with UID %d not found in session, ignoring", notificationUid);
      return;
    }

    if (notifications.contains(notificationUid)) {
      ancsNotif = notifications[notificationUid];
    } else {
      // If the Notification source didn't add it earlier, then don't process it
      NRF_LOG_INFO("Notification with UID %d not found in notifications map, ignoring datasource", notificationUid);
      return;
    }

    std::string decodedTitle = DecodeUtf8String(event->notify_rx.om, titleSize, 8);

    std::string decodedSubTitle = DecodeUtf8String(event->notify_rx.om, subTitleSize, 8 + titleSize + 1 + 2);

    std::string decodedMessage = DecodeUtf8String(event->notify_rx.om, messageSize, 8 + titleSize + 1 + 2 + subTitleSize + 1 + 2);

    // Debug event ids ands flags by putting them at front of message (in int format)
    // decodedMessage = std::to_string(ancsNotif.uuid) + " " + decodedMessage;

    NRF_LOG_INFO("Decoded Title: %s", decodedTitle.c_str());
    NRF_LOG_INFO("Decoded SubTitle: %s", decodedSubTitle.c_str());

    bool incomingCall = ancsNotif.uuid != 0 && ancsNotif.category == static_cast<uint8_t>(Categories::IncomingCall);

    if (!incomingCall) {
      if (titleSize >= maxTitleSize) {
        decodedTitle.resize(maxTitleSize - 3);
        decodedTitle += "...";
        if (!decodedSubTitle.empty()) {
          decodedTitle += " - ";
        } else {
          decodedTitle += "-";
        }
      } else {
        if (!decodedSubTitle.empty()) {
          decodedTitle += " - ";
        } else {
          decodedTitle += "-";
        }
      }

      if (subTitleSize > maxSubtitleSize) {
        decodedSubTitle.resize(maxSubtitleSize - 3);
        decodedSubTitle += "...";
      }
    }

    titleSize = static_cast<uint16_t>(decodedTitle.size());
    subTitleSize = static_cast<uint16_t>(decodedSubTitle.size());
    messageSize = static_cast<uint16_t>(decodedMessage.size());

    std::string notifStr;

    if (incomingCall) {
      notifStr += "Incoming Call:";
      notifStr += decodedTitle;
      notifStr += "\n";
      notifStr += decodedSubTitle;
    } else {
      notifStr += decodedTitle;
      if (!decodedSubTitle.empty()) {
        notifStr += decodedSubTitle + ":";
      }
      notifStr += decodedMessage;
    }

    // Adjust notification if too long
    if (notifStr.size() > NotificationManager::MessageSize) {
      notifStr.resize(97);
      notifStr += "...";
    }

    std::array<char, NotificationManager::MessageSize + 1> buffer {};
    std::strncpy(buffer.data(), notifStr.c_str(), buffer.size() - 1);

    if (incomingCall) {
      buffer[13] = '\0'; // Separate Title and Message
    } else if (!decodedSubTitle.empty()) {
      buffer[titleSize + subTitleSize] = '\0'; // Separate Title and Message
    } else {
      buffer[titleSize - 1] = '\0'; // Separate Title and Message
    }

    const auto bufferSize = static_cast<uint8_t>(std::min(notifStr.size() + 1, buffer.size()));
    NotificationManager::Notification notif(buffer.data(), bufferSize);
    notif.ancsUid = notificationUid;
    notif.icon = IconForAppId(appId.data());
    if (incomingCall) {
      notif.category = Pinetime::Controllers::NotificationManager::Categories::IncomingCall;
    } else {
      notif.category = Pinetime::Controllers::NotificationManager::Categories::SimpleAlert;
    }
    notificationManager.Push(std::move(notif));

    // Only ping the system task if the notification was added and ignore pre-existing notifications
    if (ancsNotif.isProcessed == false && (ancsNotif.eventFlags & static_cast<uint8_t>(EventFlags::Silent)) == 0 &&
        (ancsNotif.eventFlags & static_cast<uint8_t>(EventFlags::PreExisting)) == 0) {
      systemTask.PushMessage(Pinetime::System::Messages::OnNewNotification);
    }

    // Mark the notification as processed in the session
    notifications[notificationUid].isProcessed = true;
  }
}

void AppleNotificationCenterClient::AcceptIncomingCall(uint32_t uuid) {
  if (notifications.contains(uuid)) {
    const AncsNotification ancsNotif = notifications[uuid];
    if (ancsNotif.category != static_cast<uint8_t>(Categories::IncomingCall)) {
      return;
    }
  } else {
    return;
  }

  uint8_t value[6];
  value[0] = 0x02; // Command ID: Perform Notification Action
  value[1] = static_cast<uint8_t>((uuid & 0xFF));
  value[2] = static_cast<uint8_t>((uuid >> 8) & 0xFF);
  value[3] = static_cast<uint8_t>((uuid >> 16) & 0xFF);
  value[4] = static_cast<uint8_t>((uuid >> 24) & 0xFF);
  value[5] = 0x00; // Action ID: Positive Action

  ble_gattc_write_flat(systemTask.nimble().connHandle(), controlPointHandle, value, sizeof(value), OnControlPointWriteCallback, this);
}

void AppleNotificationCenterClient::RejectIncomingCall(uint32_t uuid) {
  AncsNotification ancsNotif;
  if (notifications.contains(uuid)) {
    ancsNotif = notifications[uuid];
    if (ancsNotif.category != static_cast<uint8_t>(Categories::IncomingCall)) {
      return;
    }
  } else {
    return;
  }

  uint8_t value[6];
  value[0] = 0x02; // Command ID: Perform Notification Action
  value[1] = static_cast<uint8_t>(uuid & 0xFF);
  value[2] = static_cast<uint8_t>((uuid >> 8) & 0xFF);
  value[3] = static_cast<uint8_t>((uuid >> 16) & 0xFF);
  value[4] = static_cast<uint8_t>((uuid >> 24) & 0xFF);
  value[5] = 0x01; // Action ID: Negative Action

  ble_gattc_write_flat(systemTask.nimble().connHandle(), controlPointHandle, value, sizeof(value), OnControlPointWriteCallback, this);
}

void AppleNotificationCenterClient::Reset() {
  ancsStartHandle = 0;
  ancsEndHandle = 0;
  gattStartHandle = 0;
  gattEndHandle = 0;
  serviceChangedHandle = 0;
  serviceChangedDescriptorHandle = 0;
  notificationSourceHandle = 0;
  notificationSourceDescriptorHandle = 0;
  controlPointHandle = 0;
  controlPointDescriptorHandle = 0;
  dataSourceHandle = 0;
  dataSourceDescriptorHandle = 0;
  isGattDiscovered = false;
  isGattCharacteristicDiscovered = false;
  isGattDescriptorFound = false;
  isDiscovered = false;
  isCharacteristicDiscovered = false;
  isDescriptorFound = false;
  isControlCharacteristicDiscovered = false;
  isControlDescriptorFound = false;
  isDataCharacteristicDiscovered = false;
  isDataDescriptorFound = false;
  subscriptionsDone = false;

  notifications.clear();
}

void AppleNotificationCenterClient::Discover(uint16_t connectionHandle, std::function<void(uint16_t)> onServiceDiscovered) {
  NRF_LOG_INFO("[ANCS] Starting discovery");
  this->onServiceDiscovered = onServiceDiscovered;
  ble_gattc_disc_svc_by_uuid(connectionHandle, &ancsUuid.u, OnDiscoveryEventCallback, this);
}

// This function is used for debugging purposes to log a message and push a notification
// Used to test BLE debugging on production devices
void AppleNotificationCenterClient::DebugNotification(const char* msg) const {
  NRF_LOG_INFO("[ANCS DEBUG] %s", msg);

  NotificationManager::Notification notif(msg,
                                          static_cast<uint8_t>(std::min<size_t>(std::strlen(msg) + 1, NotificationManager::MessageSize)));
  notif.category = Pinetime::Controllers::NotificationManager::Categories::SimpleAlert;
  notificationManager.Push(std::move(notif));

  systemTask.PushMessage(Pinetime::System::Messages::OnNewNotification);
}

std::string AppleNotificationCenterClient::DecodeUtf8String(os_mbuf* om, uint16_t size, uint16_t offset) {
  std::string decoded;
  decoded.reserve(size);

  auto isInFontDefinition = [](uint32_t codepoint) -> bool {
    // Check if the codepoint falls into the specified font ranges or is explicitly listed
    return (codepoint >= 0x20 && codepoint <= 0x7E) ||   // Printable ASCII
           (codepoint >= 0x410 && codepoint <= 0x44F) || // Cyrillic
           codepoint == 0xB0 ||
           // Extra characters taken from Titus Kendzorra's InfiniBros fork. Keep in sync with fonts.json.
           codepoint == 0xC4 || codepoint == 0xD6 || codepoint == 0xDC || codepoint == 0xDF || // Ä Ö Ü ß
           codepoint == 0xE4 || codepoint == 0xF6 || codepoint == 0xFC ||                      // ä ö ü
           codepoint == 0x20AC ||                                                              // €
           codepoint == 0x2018 || codepoint == 0x2019 ||                                       // ‘ ’ (iOS smart apostrophes)
           codepoint == 0x201C || codepoint == 0x201D || codepoint == 0x201E ||                // “ ” „
           codepoint == 0x2022 || codepoint == 0x2026;                                         // • …
  };

  // Invisible characters that change how the emoji before them looks. Drop them instead of showing �.
  auto isEmojiModifier = [](uint32_t codepoint) -> bool {
    return codepoint == 0xFE0F ||                          // Variation selector, e.g. the one after ❤
           codepoint == 0x200D ||                          // Zero-width joiner
           (codepoint >= 0x1F3FB && codepoint <= 0x1F3FF); // Skin tones
  };

  for (uint16_t i = 0; i < size;) {
    uint8_t byte = 0;
    if (os_mbuf_copydata(om, offset + i, 1, &byte) != 0) {
      break; // Handle error in copying data (e.g., log or terminate processing)
    }

    if (byte <= 0x7F) { // Single-byte UTF-8 (ASCII)
      if (isInFontDefinition(byte)) {
        decoded.push_back(static_cast<char>(byte));
      } else {
        decoded.append("�"); // Replace unsupported
      }
      ++i;
    } else { // Multi-byte UTF-8
      // Determine sequence length based on leading byte
      int sequenceLength = 0;
      if ((byte & 0xE0) == 0xC0) {
        sequenceLength = 2; // 2-byte sequence
      } else if ((byte & 0xF0) == 0xE0) {
        sequenceLength = 3; // 3-byte sequence
      } else if ((byte & 0xF8) == 0xF0) {
        sequenceLength = 4; // 4-byte sequence
      }

      if (i + sequenceLength > size) {
        decoded.append("�"); // Incomplete sequence, replace
        break;
      }

      // Read the full sequence
      std::string utf8Char;
      bool validSequence = true;
      uint32_t codepoint = 0;

      for (int j = 0; j < sequenceLength; ++j) {
        uint8_t nextByte = 0;
        os_mbuf_copydata(om, offset + i + j, 1, &nextByte);
        utf8Char.push_back(static_cast<char>(nextByte));

        if (j == 0) {
          // Leading byte contributes significant bits
          if (sequenceLength == 2) {
            codepoint = nextByte & 0x1F;
          } else if (sequenceLength == 3) {
            codepoint = nextByte & 0x0F;
          } else if (sequenceLength == 4) {
            codepoint = nextByte & 0x07;
          }
        } else {
          // Continuation bytes contribute lower bits
          if ((nextByte & 0xC0) != 0x80) {
            validSequence = false; // Invalid UTF-8 continuation byte
            break;
          }
          codepoint = (codepoint << 6) | (nextByte & 0x3F);
        }
      }

      const char* emojiSymbol = validSequence ? SymbolForEmoji(codepoint) : nullptr;
      if (validSequence && isInFontDefinition(codepoint)) {
        decoded.append(utf8Char); // Append valid UTF-8 character
      } else if (emojiSymbol != nullptr) {
        decoded.append(emojiSymbol); // Emoji drawn with a Font Awesome glyph
      } else if (validSequence && isEmojiModifier(codepoint)) {
        // Skip
      } else {
        decoded.append("�"); // Replace unsupported
      }
      i += sequenceLength;
    }
  }

  return decoded;
}
