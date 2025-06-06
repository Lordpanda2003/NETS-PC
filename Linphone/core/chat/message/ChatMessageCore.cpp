/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
 *
 * This file is part of linphone-desktop
 * (see https://www.linphone.org).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ChatMessageCore.hpp"
#include "core/App.hpp"
#include "core/chat/ChatCore.hpp"
#include "model/tool/ToolModel.hpp"

DEFINE_ABSTRACT_OBJECT(ChatMessageCore)

QSharedPointer<ChatMessageCore> ChatMessageCore::create(const std::shared_ptr<linphone::ChatMessage> &chatmessage) {
	auto sharedPointer = QSharedPointer<ChatMessageCore>(new ChatMessageCore(chatmessage), &QObject::deleteLater);
	sharedPointer->setSelf(sharedPointer);
	sharedPointer->moveToThread(App::getInstance()->thread());
	return sharedPointer;
}

ChatMessageCore::ChatMessageCore(const std::shared_ptr<linphone::ChatMessage> &chatmessage) {
	// lDebug() << "[ChatMessageCore] new" << this;
	mustBeInLinphoneThread(getClassName());
	App::getInstance()->mEngine->setObjectOwnership(this, QQmlEngine::CppOwnership);
	mChatMessageModel = Utils::makeQObject_ptr<ChatMessageModel>(chatmessage);
	mChatMessageModel->setSelf(mChatMessageModel);
	mText = mChatMessageModel->getText();
	mTimestamp = QDateTime::fromSecsSinceEpoch(chatmessage->getTime());
	auto from = chatmessage->getFromAddress();
	auto to = chatmessage->getLocalAddress();
	mIsRemoteMessage = !from->weakEqual(to);
	mPeerAddress = Utils::coreStringToAppString(chatmessage->getPeerAddress()->asStringUriOnly());
	mPeerName = ToolModel::getDisplayName(chatmessage->getPeerAddress()->clone());
	auto fromAddress = chatmessage->getFromAddress()->clone();
	fromAddress->clean();
	mFromAddress = Utils::coreStringToAppString(fromAddress->asStringUriOnly());
	mFromName = ToolModel::getDisplayName(chatmessage->getFromAddress()->clone());

	auto chatroom = chatmessage->getChatRoom();
	mIsFromChatGroup = chatroom->hasCapability((int)linphone::ChatRoom::Capabilities::Conference) &&
	                   !chatroom->hasCapability((int)linphone::ChatRoom::Capabilities::OneToOne);
	mIsRead = chatmessage->isRead();
	mMessageState = LinphoneEnums::fromLinphone(chatmessage->getState());
}

ChatMessageCore::~ChatMessageCore() {
}

void ChatMessageCore::setSelf(QSharedPointer<ChatMessageCore> me) {
	mChatMessageModelConnection = SafeConnection<ChatMessageCore, ChatMessageModel>::create(me, mChatMessageModel);
	mChatMessageModelConnection->makeConnectToCore(&ChatMessageCore::lDelete, [this] {
		mChatMessageModelConnection->invokeToModel([this] { mChatMessageModel->deleteMessageFromChatRoom(); });
	});
	mChatMessageModelConnection->makeConnectToModel(&ChatMessageModel::messageDeleted, [this]() {
		Utils::showInformationPopup(tr("Supprimé"), tr("Message supprimé"), true);
		emit deleted();
	});
	mChatMessageModelConnection->makeConnectToCore(&ChatMessageCore::lMarkAsRead, [this] {
		mChatMessageModelConnection->invokeToModel([this] { mChatMessageModel->markAsRead(); });
	});
	mChatMessageModelConnection->makeConnectToModel(&ChatMessageModel::messageRead, [this]() { setIsRead(true); });

	mChatMessageModelConnection->makeConnectToModel(
	    &ChatMessageModel::msgStateChanged,
	    [this](const std::shared_ptr<linphone::ChatMessage> &message, linphone::ChatMessage::State state) {
		    if (mChatMessageModel->getMonitor() != message) return;
		    auto msgState = LinphoneEnums::fromLinphone(state);
		    mChatMessageModelConnection->invokeToCore([this, msgState] { setMessageState(msgState); });
	    });
}

QDateTime ChatMessageCore::getTimestamp() const {
	return mTimestamp;
}

void ChatMessageCore::setTimestamp(QDateTime timestamp) {
	if (mTimestamp != timestamp) {
		mTimestamp = timestamp;
		emit timestampChanged(timestamp);
	}
}

QString ChatMessageCore::getText() const {
	return mText;
}

void ChatMessageCore::setText(QString text) {
	if (mText != text) {
		mText = text;
		emit textChanged(text);
	}
}

QString ChatMessageCore::getPeerAddress() const {
	return mPeerAddress;
}

QString ChatMessageCore::getPeerName() const {
	return mPeerName;
}

QString ChatMessageCore::getFromAddress() const {
	return mFromAddress;
}

QString ChatMessageCore::getFromName() const {
	return mFromName;
}

QString ChatMessageCore::getToAddress() const {
	return mToAddress;
}

bool ChatMessageCore::isRemoteMessage() const {
	return mIsRemoteMessage;
}

bool ChatMessageCore::isFromChatGroup() const {
	return mIsFromChatGroup;
}

bool ChatMessageCore::isRead() const {
	return mIsRead;
}

void ChatMessageCore::setIsRead(bool read) {
	if (mIsRead != read) {
		mIsRead = read;
		emit isReadChanged(read);
	}
}

LinphoneEnums::ChatMessageState ChatMessageCore::getMessageState() const {
	return mMessageState;
}

void ChatMessageCore::setMessageState(LinphoneEnums::ChatMessageState state) {
	if (mMessageState != state) {
		mMessageState = state;
		emit messageStateChanged();
	}
}

std::shared_ptr<ChatMessageModel> ChatMessageCore::getModel() const {
	return mChatMessageModel;
}