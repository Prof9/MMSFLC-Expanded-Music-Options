#include <Enums_Internal.hpp>

#include <map>
#include <vector>

#include "Guid.hpp"
#include "MusicSystemMod.hpp"

const std::map<Guid, std::map<via::Language, char16_t const *>> MusicSystemMod::STRINGS =
	{
		{L"8debd0e1-ad1f-4afc-80a7-d1013ad86e89"_guid,
		 {
			 {via::Language::English, u"Real World BGM"},
		 }},
		{L"c80c7d4e-fef6-4a17-839d-c983ceaafee8"_guid,
		 {
			 {via::Language::English, u"Select what tracks will play in the Real World."},
		 }},
		{L"9d3c9c45-c53f-42dd-b2fc-1beebdf63a28"_guid,
		 {
			 {via::Language::English, u"Wave Road BGM"},
		 }},
		{L"8c8efb06-f15d-4844-baea-1ea64ebba80a"_guid,
		 {
			 {via::Language::English, u"Select what tracks will play on Wave Roads."},
		 }},
		{L"ac468c10-eb46-4101-8dcf-922c9407a09a"_guid,
		 {
			 {via::Language::English, u"Wave Space BGM"},
		 }},
		{L"bb07dbc0-d6df-4719-b32c-ecf9f2eb7ccd"_guid,
		 {
			 {via::Language::English, u"Select what tracks will play in Comp Spaces, EM Wave Spaces,\nCyber Cores, and non-numbered Noise Waves."},
		 }},
		{L"00214dd1-02cf-46be-9d60-7137604c6daa"_guid,
		 {
			 {via::Language::English, u"Dungeon BGM"},
		 }},
		{L"b91d4a7b-c8d9-46b8-ae1e-e9cb1d632015"_guid,
		 {
			 {via::Language::English, u"Select what tracks will play in dungeons, the Sky Wave,\nthe Astro Wave, and numbered Noise Waves."},
		 }},
		{L"ad4fcc9e-eccf-4e64-8afb-2286132ca680"_guid,
		 {
			 {via::Language::English, u"Normal Battle BGM"},
		 }},
		{L"13dec11f-c01f-492c-bd69-c7b0da9fb32f"_guid,
		 {
			 {via::Language::English, u"Select what tracks will play in regular battles and online Matches."},
		 }},
		{L"a95b7459-663c-4fa6-af8d-a3294250efbc"_guid,
		 {
			 {via::Language::English, u"Boss Battle BGM"},
		 }},
		{L"f9548ad4-63c6-44e3-9291-a2d0000ad2b7"_guid,
		 {
			 {via::Language::English, u"Select what tracks will play in boss battles."},
		 }},
		{L"b9144bd0-98da-439a-9da4-021c5365d153"_guid,
		 {
			 {via::Language::English, u"Last Battle BGM"},
		 }},
		{L"a14c26cf-289d-4c1a-a8bc-33735eb110a6"_guid,
		 {
			 {via::Language::English, u"Select what tracks will play in the final battle."},
		 }},
		{L"6fb1bc61-3b8b-4826-a409-edc9e374f200"_guid,
		 {
			 {via::Language::English, u"Winner! BGM"},
		 }},
		{L"c89aa7f5-56ec-4dc1-b7e0-29a60836e0e7"_guid,
		 {
			 {via::Language::English, u"Select what tracks will play at the end of most won battles.\n(Some options will not apply to battles without rewards.)"},
		 }},
		{L"3744b966-9ad0-4267-bb8e-a961a0f146eb"_guid,
		 {
			 {via::Language::English, u"Loser... BGM"},
		 }},
		{L"bd4eee29-1654-46ea-b94d-1ebcec877a4f"_guid,
		 {
			 {via::Language::English, u"Select what tracks will play at the end of lost battles in online Matches."},
		 }},
		{L"5e8401f5-8a28-4efb-9311-2e8fc505d79e"_guid,
		 {
			 {via::Language::English, u"Incident BGM (Real World)"},
		 }},
		{L"115ddb26-3a7a-4e90-8b60-ce581697d8ef"_guid,
		 {
			 {via::Language::English, u"Select what tracks will play during incidents in the Real World."},
		 }},
		{L"6dd57050-d840-4f5c-bda4-8868441dc1aa"_guid,
		 {
			 {via::Language::English, u"Incident BGM (Wave World)"},
		 }},
		{L"9074ac1f-c5c5-4776-a97b-83066bf0b07c"_guid,
		 {
			 {via::Language::English, u"Select what tracks will play during incidents in the Wave World."},
		 }},
		{L"65c3c7c7-fa93-45fa-9cbf-f3a4cf23b862"_guid,
		 {
			 {via::Language::English, u"Miscellaneous BGM"},
		 }},
		{L"10428196-f3b5-452f-94f8-50e8a38139ce"_guid,
		 {
			 {via::Language::English, u"Select what tracks will play during miscellaneous events."},
		 }},
		{L"8a38a4b9-0527-4b36-abde-c96ca132db30"_guid,
		 {
			 {via::Language::English, u"Title Screen BGM"},
		 }},
		{L"69a9799a-898f-40c8-b1e0-550eb37256e7"_guid,
		 {
			 {via::Language::English, u"Select what tracks will play on the in-game title screen."},
		 }},
		{L"3cc5ab6c-4a2a-4cb0-8816-845aefbf01ab"_guid,
		 {
			 {via::Language::English, u"Main Menu BGM"},
		 }},
		{L"bc71a2d1-54df-41b0-a2ee-670a4eb6e0a9"_guid,
		 {
			 {via::Language::English, u"Select what tracks will play on the collection's Main Menu."},
		 }},
		{L"73c512c7-6d5d-405e-96d8-d9bcc7eff8d7"_guid,
		 {
			 {via::Language::English, u"DLC BGM - Loneliness"},
		 }},
		{L"238a92fc-c37b-4319-b2e1-f40cb6915c33"_guid,
		 {
			 {via::Language::English, u"Select whether to play Loneliness - Summer Shade Version in-game.\n(This takes priority over the above settings.)"},
		 }},
		{L"28a49ef5-fc0c-48c5-a8eb-60c92ba20cc8"_guid,
		 {
			 {via::Language::English, u"DLC BGM - Astro Wave"},
		 }},
		{L"5491219b-1b6e-4e37-a894-e065e37d7bb2"_guid,
		 {
			 {via::Language::English, u"Select whether to play Astro Wave - Brilliant Expanse Version in-game.\n(This takes priority over the above settings.)"},
		 }},
		{L"cc097b76-9d12-40db-91ee-dff3e0852892"_guid,
		 {
			 {via::Language::English, u"DLC BGM - Anthem of the Solitary"},
		 }},
		{L"8313007b-6258-4f73-82e3-07952e8d9831"_guid,
		 {
			 {via::Language::English, u"Select whether to play Anthem of the Solitary - Howling Void Version in-game.\n(This takes priority over the above settings.)"},
		 }},
		{L"31d45ae4-128e-4172-820f-4fd2b06510a5"_guid,
		 {
			 {via::Language::English, u"DLC BGM - Cheerful Indoors"},
		 }},
		{L"61511466-dd94-4c00-ae88-4a8878002c8b"_guid,
		 {
			 {via::Language::English, u"Select whether to play Cheerful Indoors - School Days Version in-game.\n(This takes priority over the above settings.)"},
		 }},
		{L"c564c411-216f-498f-9c53-459408c675d5"_guid,
		 {
			 {via::Language::English, u"No Change"},
		 }},
		{L"efab3c28-9b48-442f-9fcb-7cf75b46b7bc"_guid,
		 {
			 {via::Language::English, u"Play the normal track as determined by BGM Selection."},
		 }},
		{L"33fffdce-f115-4cd5-ade2-2a168f47d19a"_guid,
		 {
			 {via::Language::English, u"Music Off"},
		 }},
		{L"7f561d13-08c8-43c5-af02-4248afa489e4"_guid,
		 {
			 {via::Language::English, u"Background music is disabled for this type of track."},
		 }},
		{L"b46d7983-3acf-40a4-8c71-3f2bfb7d7b89"_guid,
		 {
			 {via::Language::English, u"Always Original"},
		 }},
		{L"bf4b7003-90ae-45fa-bcda-66d72fa818e5"_guid,
		 {
			 {via::Language::English, u"Always play the original version of the track."},
		 }},
		{L"773ac3ba-06fc-489c-85f1-589efb6b546f"_guid,
		 {
			 {via::Language::English, u"Always Arranged"},
		 }},
		{L"7fd7bc1a-88ee-4631-bbec-a731d86deed5"_guid,
		 {
			 {via::Language::English, u"Always play the arranged version of the track."},
		 }},
		{L"c12ba05a-7bbc-4e46-b510-53a361c262f4"_guid,
		 {
			 {via::Language::English, u"Preferred Mix <ICON LAUNCHER_DECIDE>"},
		 }},
		{L"501e2a85-77ba-4f87-a7a1-b672e5674e54"_guid,
		 {
			 {via::Language::English, u"Always play the original or arranged track if that version is marked as Favorite.\n(This list is shared for all track types.)"},
		 }},
		{L"36fac9cf-514b-45f7-b015-b5b54b4817c3"_guid,
		 {
			 {via::Language::English, u"Playlist <ICON LAUNCHER_DECIDE>"},
		 }},
		{L"297584f6-5da9-4131-99fd-365f2d8746ac"_guid,
		 {
			 {via::Language::English, u"Always play tracks randomly selected from a\ndedicated list of Favorites for this type of track."},
		 }},
		{L"c5451ab5-0f50-45c0-8368-de71c13aa44b"_guid,
		 {
			 {via::Language::English, u"Favorites <ICON LAUNCHER_DECIDE>"},
		 }},
		{L"99b3bbca-167c-416c-aa9d-1349dac7b784"_guid,
		 {
			 {via::Language::English, u"Always play tracks randomly selected from your regular list of Favorites."},
		 }},
		{L"0c174efc-4ff0-42ea-98d2-c0956ca6c5ba"_guid,
		 {
			 {via::Language::English, u"Field"},
		 }},
		{L"53ff37ca-a276-470c-b310-8c0ad3ce1349"_guid,
		 {
			 {via::Language::English, u"Play the background music from the Field Screen uninterrupted.\n(Does not apply to online Matches.)"},
		 }},
		{L"c7e77ba9-a882-4d37-a11b-e5900d140d51"_guid,
		 {
			 {via::Language::English, u"Field Original"},
		 }},
		{L"18b51d97-8636-4bb6-b6b7-4e5a2c46afd5"_guid,
		 {
			 {via::Language::English, u"Play the original version of the background music from the Field Screen uninterrupted.\n(Does not apply to online Matches.)"},
		 }},
		{L"91937c25-8452-45d8-a735-aa569695badb"_guid,
		 {
			 {via::Language::English, u"Field Arranged"},
		 }},
		{L"bdbd21a1-9136-4639-bf6a-bda1fdc18e64"_guid,
		 {
			 {via::Language::English, u"Play the arranged version of the background music from the Field Screen uninterrupted.\n(Does not apply to online Matches.)"},
		 }},
		{L"0a1089a5-f79f-45d4-9329-f159d2d4fdc2"_guid,
		 {
			 {via::Language::English, u"Battle"},
		 }},
		{L"22089bb3-1ed9-498f-bb7b-b10063cdc380"_guid,
		 {
			 {via::Language::English, u"Play the background music from the Battle Screen uninterrupted."},
		 }},
		{L"079c9622-16d9-4871-b199-c25eb73196c8"_guid,
		 {
			 {via::Language::English, u"<ICON LAUNCHER_DECIDE> Edit List"},
		 }},
};