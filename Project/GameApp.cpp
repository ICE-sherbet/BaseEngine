/*************************************************************************//*!
					
					@file	GameApp.cpp
					@brief	基本ゲームアプリ。

															@author	濱田　享
															@date	2014.05.14
*//**************************************************************************/

// INCLUDE
#include "GameApp.h"

#include <array>
#include <fstream>


#include "TileMap.h"
#include "BinaryArchive.h"
#include "VectorFrozen.h"
#include "StringFrozen.h"
struct Test {
  std::vector<int> b{};
  std::string str{};
  template <class Archive>
  void FROZEN_SERIALIZE_FUNCTION_NAME(Archive& archive) {
    archive(b, str);
  }
};
struct Test2 {
  std::vector<int> b{};
  std::string str{};
  Test t;
  template <class Archive>
  void FROZEN_SERIALIZE_FUNCTION_NAME(Archive& archive) {
    archive(b, str,t);
  }
};

struct Test3 {
  std::vector<int> b{};
  std::string str{};
};
  struct Test4 {
  std::vector<int> b{};
  std::string str{};
  Test3 t3;
  template <class Archive>
  void FROZEN_SERIALIZE_FUNCTION_NAME(Archive& archive) {
    archive(b, str, t3);
  }
};
/*************************************************************************//*!
		@brief			アプリケーションの初期化
		@param			None
				
		@return			TRUE		成功<br>
						それ以外	失敗、エラーコードが戻り値となる
*//**************************************************************************/
MofBool CGameApp::Initialize(void) {
  CUtilities::SetCurrentDirectory("Resource");
  // CGraphicsUtilities::SetCamera(&camera_);

  g_pGraphics->SetCullMode(CULLMODE_NONE);
  //ShowCursor(false);
  game_.Initialize();
  return TRUE;
}
/*************************************************************************//*!
		@brief			アプリケーションの更新
		@param			None
				
		@return			TRUE		成功<br>
						それ以外	失敗、エラーコードが戻り値となる
*//**************************************************************************/
MofBool CGameApp::Update(void) {
  //キーの更新
  g_pInput->RefreshKey();
  game_.Update();
  if (g_pInput->IsKeyPush(MOFKEY_DELETE))
  {
    PostQuitMessage(0);
  }

  return TRUE;
}
/*************************************************************************//*!
		@brief			アプリケーションの描画
		@param			None
				
		@return			TRUE		成功<br>
						それ以外	失敗、エラーコードが戻り値となる
*//**************************************************************************/
MofBool CGameApp::Render(void) {
  //描画開始
  g_pGraphics->RenderStart();
  //画面のクリア
  g_pGraphics->ClearTarget(0.2f, 0.2f, 0.2f, 0.0f, 1.0f, 0);
  game_.Render();
  //描画の終了
  g_pGraphics->RenderEnd();
  return TRUE;
}
/*************************************************************************//*!
		@brief			アプリケーションの解放
		@param			None
				
		@return			TRUE		成功<br>
						それ以外	失敗、エラーコードが戻り値となる
*//**************************************************************************/
MofBool CGameApp::Release(void) {
  game_.Shutdown();
  g_pGraphics->SetScreenMode(true);
  return TRUE;
}