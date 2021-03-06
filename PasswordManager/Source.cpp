#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/checkbox.hpp>
#include <string>
#include <vector>
#include <mutex>
#include <iostream>
#include <Windows.h>
#include <sstream>

static int keypass = 8;


std::string encrypt(std::string z) {
	char * s = (char *)z.c_str();
	char * tmp = (char*)malloc(z.length() * sizeof(char));
	int a = z.length();
	__asm {
		mov ecx, a;
		inc ecx
		mov esi, s;
		mov edi, tmp;
	copy_loop:
		lodsb;
		stosb;
		loop copy_loop
	}
	__asm {
		movzx ebx, keypass;
		mov ecx, a;
		mov esi, tmp;
		mov edi, tmp;
	encrypt_loop:
		lodsb;
		add al, bl;
		stosb;
		loop encrypt_loop;
	}
	return std::string(tmp);
}
std::string decrypt(std::string z) {
	char * s = (char *)z.c_str();
	char * tmp = (char*)malloc(z.length() * sizeof(char));
	int a = z.length();
	__asm {
		mov ecx, a;
		inc ecx
		mov esi, s;
		mov edi, tmp;
	copy_loop:
		lodsb;
		stosb;
		loop copy_loop
	}
	__asm {
		movzx ebx, keypass;
		mov ecx, a;
		mov esi, tmp;
		mov edi, tmp;
	decrypt_loop:
		lodsb;
		sub al, bl;
		stosb;
		loop decrypt_loop;
	}
	return std::string(tmp);
}

using namespace nana;

//Creates a textbox and button
//textbox shows the value of the sub item
//button is used to delete the item.
class inline_widget2 : public listbox::inline_notifier_interface {
private:
	//Creates inline widget
	//listbox calls this method to create the widget
	//The position and size of widget can be ignored in this process
	virtual void create(window wd) override {
		//Create listbox
		txt_.create(wd);
		txt_.events().click([this] {
			//Select the item when clicks the textbox
			indicator_->selected(pos_);
		});

		txt_.events().mouse_move([this] {
			//Highlight the item when hovers the textbox
			indicator_->hovered(pos_);
		});

		txt_.events().key_char([this](const arg_keyboard& arg) {
			if (arg.key == keyboard::enter) {
				//Modify the item when enter is pressed
				arg.ignore = true;
				indicator_->modify(pos_, txt_.caption());
			}
		});
		//Or modify the item when typing
		txt_.events().text_changed([this]() {
			indicator_->modify(pos_, txt_.caption());
		});
		//Create button
		btn_.create(wd);
		//btn_.check(1);
		//btn_.caption("Delete");
		btn_.events().checked([this] {

			if (!this->btn_.checked()) { 
				indicator_->modify(pos_, encrypt(txt_.caption()));
				txt_.editable(0);
			}
			if (this->btn_.checked()) {
				txt_.editable(1);
				indicator_->modify(pos_, decrypt(txt_.caption()));
			}

		});

		btn_.events().mouse_move([this] {
			//Highlight the item when hovers the button
			indicator_->hovered(pos_);
		});
		checkboxes.push_back(&btn_);
	}

	//Activates the inline widget, bound to a certain item of the listbox
	//The inline_indicator is an object to operate the listbox item,
	//pos is an index object refers to the item of listbox
	virtual void activate(inline_indicator& ind, index_type pos) {
		indicator_ = &ind;
		pos_ = pos;
	}

	void notify_status(status_type status, bool status_on) override {
		//Sets focus for the textbox when the item is selected
		if ((status_type::selecting == status) && status_on)
			txt_.focus();
	}

	//Sets the inline widget size
	//dimension represents the max size can be set
	//The coordinate of inline widget is a logical coordinate to the sub item of listbox
	void resize(const size& dimension) override {
		auto sz = dimension;
		sz.width -= (sz.width < 30 ? 0 : 30); //Check for avoiding underflow.
		txt_.size(sz);

		rectangle r(sz.width + 5, 0, 45, sz.height);
		btn_.move(r);
		//rectangle t(sz.width, 5, 100, sz.height);
		//chb_.move(t);
	}

	//Sets the value of inline widget with the value of the sub item
	virtual void set(const value_type& value) {
		//Avoid emitting text_changed to set caption again, otherwise it
		//causes infinite recursion.
		if (txt_.caption() != value) {
			if (!en_first) {
				txt_.caption(encrypt(value));
				en_first = true;
				txt_.editable(0);
			} else {
				txt_.caption(value);
			}
		}
			
	}

	//Determines whether to draw the value of sub item
	//e.g, when the inline widgets covers the whole background of the sub item,
	//it should return false to avoid listbox useless drawing
	bool whether_to_draw() const override {
		return false;
	}
public:
	inline_indicator * indicator_{ nullptr };
	index_type pos_;
	textbox txt_;
	checkbox btn_;
	bool en_first = false;
	static std::vector<checkbox *> checkboxes;
};

class inline_widget : public listbox::inline_notifier_interface {
private:
	//Creates inline widget
	//listbox calls this method to create the widget
	//The position and size of widget can be ignored in this process
	virtual void create(window wd) override {
		//Create listbox
		txt_.create(wd);
		txt_.events().click([this] {
			//Select the item when clicks the textbox
			indicator_->selected(pos_);
		});

		txt_.events().mouse_move([this] {
			//Highlight the item when hovers the textbox
			indicator_->hovered(pos_);
		});

		txt_.events().key_char([this](const arg_keyboard& arg) {
			if (arg.key == keyboard::enter) {
				//Modify the item when enter is pressed
				arg.ignore = true;
				indicator_->modify(pos_, txt_.caption());
			}
		});
		//Or modify the item when typing
		txt_.events().text_changed([this]() {
			indicator_->modify(pos_, txt_.caption());
		});
		//Create button
		btn_.create(wd);
		btn_.caption("Delete");
		btn_.events().click([this] {
			//Delete the item when button is clicked
			inline_widget2::checkboxes.pop_back();
			auto & lsbox = dynamic_cast<listbox&>(indicator_->host());
			lsbox.erase(lsbox.at(pos_));
		});

		btn_.events().mouse_move([this] {
			//Highlight the item when hovers the button
			indicator_->hovered(pos_);
		});
	}

	//Activates the inline widget, bound to a certain item of the listbox
	//The inline_indicator is an object to operate the listbox item,
	//pos is an index object refers to the item of listbox
	virtual void activate(inline_indicator& ind, index_type pos) {
		indicator_ = &ind;
		pos_ = pos;
	}

	void notify_status(status_type status, bool status_on) override {
		//Sets focus for the textbox when the item is selected
		if ((status_type::selecting == status) && status_on)
			txt_.focus();
	}

	//Sets the inline widget size
	//dimension represents the max size can be set
	//The coordinate of inline widget is a logical coordinate to the sub item of listbox
	void resize(const size& dimension) override {
		auto sz = dimension;
		sz.width -= (sz.width < 50 ? 0 : 50); //Check for avoiding underflow.
		txt_.size(sz);

		rectangle r(sz.width + 5, 0, 45, sz.height);
		btn_.move(r);
		//rectangle t(sz.width, 5, 100, sz.height);
		//chb_.move(t);
	}

	//Sets the value of inline widget with the value of the sub item
	virtual void set(const value_type& value) {
		//Avoid emitting text_changed to set caption again, otherwise it
		//causes infinite recursion.
		if (txt_.caption() != value)
			txt_.caption(value);
	}

	//Determines whether to draw the value of sub item
	//e.g, when the inline widgets covers the whole background of the sub item,
	//it should return false to avoid listbox useless drawing
	bool whether_to_draw() const override {
		return false;
	}
public:
	inline_indicator * indicator_{ nullptr };
	index_type pos_;
	textbox txt_;
	button btn_;
};

std::vector<checkbox *> inline_widget2::checkboxes;//= *new std::vector<checkbox *>();

struct data {
	std::basic_string<char> username;
	std::basic_string<char> password;
};

int main(int argc, char** argv) {
	std::vector<data> datas;
	std::string fname;
	form init;
	init.events().expose([&] {
		inputbox::text pass("Passkey");
		inputbox::text fn("Filename");
		inputbox passkey(init, "Please enter a number as your key and your filename", "Pass key and Filename");
		passkey.verify([&pass,&fn](nana::window handle) {
			if (pass.value().empty()) {
				msgbox mb(handle, "Invalid Input");
				mb << L"Passkey is required";
				mb.show();
				return false;
			}
			if (fn.value().empty()) {
				msgbox mb(handle, "Invalid Input");
				mb << L"Passkey is required";
				mb.show();
				return false;
			}
			for (char a : pass.value()) {
				if (a < '0' || a > '9') {
					msgbox mb(handle, "Invalid Input");
					mb << L"Passkey must contain integer only";
					mb.show();
					return false;
				}
			}
			return true;
		});

		if (passkey.show(pass,fn)) {
			keypass = std::stoi(pass.value()) % 7 + 1;
			fname = fn.value();
			char * lpFileName = (char*)fname.c_str();
			char * lpReOpenBuff = (char*)malloc(200 * sizeof(char));
			char * fileio_buffer = (char*)malloc(100000 * sizeof(char));
			int fSize = 0;
			int ifExist = 0;
			__asm {
				push 0x00004000;/* ustyle Use this to test for the existence of a file*/
				push lpReOpenBuff
				push lpFileName
				call OpenFile
				pushad
				call GetLastError
				cmp eax, 0; file already exist
				je filer_exist_handle
				cmp eax, 2
				je filer_not_found_handle
			filer_exist_handle :
				popad;
				xor eax, eax
				push eax; 7 param - Handle
				push 128; 6 param - /*FILE_ATTRIBUTE_NORMAL*/
				push 4; 5 param - /*OPEN_ALWAYS*/
				push eax; 4 param - Default security
				push 0x00000002; 3 param - share write
				push 0x10000000; 2 param - Generic All access
				push lpFileName; 1 param - FileName
				call CreateFileA;

				pushad;
				push 0; lpFileSizeHigh
				push eax;
				call GetFileSize;
				mov fSize, eax;
				inc ifExist;
				popad;
				pushad

				push 0; LPOVERLAPPED
				push 0; /*NULL if */an asynchronous operation
				push 1024; maximum number of bytes to be read
				push fileio_buffer; buffer that receives the data
				push eax; hfile
				call ReadFile;
				popad
				push eax
				call CloseHandle;
				jmp _there;
			filer_not_found_handle:
				popad;
				xor eax, eax;
				push eax; 7 param - Handle
				push 128; 6 param - /*FILE_ATTRIBUTE_NORMAL*/
				push 2; 5 param - /*CREATE_ALWAYS*/
				push eax; 4 param - Default security
				push 0x00000002; 3 param - share write
				push 0x10000000; 2 param - Generic All access
				push lpFileName; 1 param - FileName
				call CreateFileA;
				push eax
				call CloseHandle;
			_there:
			}

			if (ifExist) {
				__asm {
					movzx ebx, keypass;
					rol bl, 4;
					mov ecx, 1024;
					mov esi, fileio_buffer;
					mov edi, fileio_buffer;
				encrypt_loop:
					lodsb;
					xor al, bl;
					stosb;
					loop encrypt_loop;
				}
				//char* realbuf = (char*)malloc(fSize * sizeof(char));
				std::stringstream tmp(fileio_buffer);
				int length;
				tmp >> length;
				for (int i = 0; i < length/2; i++) {
					std::string u;
					std::string p;

					std::getline(tmp, u, (char)7);
					std::getline(tmp, p, (char)7);
					datas.push_back({ u,decrypt(p) });
				}
			}

		} else {
			msgbox mb(init, "Invalid Input");
			mb << L"Cannot continue without passkey";
			mb.show();
			init.close();
			std::exit(1);
		}
		init.close();
	});
	init.show();

	////Define a form.
	form fm;
	fm.caption("Password Manager");

	//Then append items
	//datas.push_back(data{ "Hello0", "World0" });
	//datas.push_back(data{ "Hello1", "World1" });
	//datas.push_back(data{ "Hello2", "World2" });
	//datas.push_back(data{ "Hello3", "World3" });
	//datas.push_back(data{ "Hello3", "World3" });




	//Define a button and answer the click event.
	button qut{ fm, "Quit" };
	qut.events().click([&fm] {
		fm.close();
	});
	

	listbox lsbox{ fm,rectangle{ 0, 0, 500, 400 } };//(fm, rectangle{ 0, 0, 500, 400 });
	lsbox.auto_draw(true);

	//Create two columns
	//lsbox.checkable(1);
	lsbox.append_header("Username");
	lsbox.append_header("Password");
	//lsbox.at(0).append({ "sdvjsb","vvbua" });
	
	auto value_translator = [](const std::vector<nana::listbox::cell>& cells) {
		data p;
		//char * u = (char*)malloc(cells[0].text.size());
		//int u_len = cells[0].text.size();
		p.username=cells[0].text;
		p.password=cells[1].text;
		//auto pu = p.username.data();
		//__asm {/*
		//	mov ecx, u_len
		//	lea esi, sou; u
		//	lea edi, u; sou
		//	rep movsb */
		//	mov ecx, sou;
		//	mov pu, ecx;
		//}
		//char * ps = (char*)malloc(cells[1].text.size());
		//u = (char*)malloc(cells[1].text.size());
		//u_len = cells[1].text.size();
		//auto sou1 = &cells[1].text[0];
		//auto pp = p.password.data();
		//__asm {/*
		//	mov ecx, u_len
		//	lea esi, sou; ps
		//	lea edi, ps; sou
		//	rep movsb*/
		//	mov ecx, sou1;
		//	mov pp,ecx
		//}
		//p.username = u;
		//p.password = ps;//std::stoul(cells[1].text);
		return p;
	};
	auto cell_translator = [](const data& p) {
		std::vector<nana::listbox::cell> cells;
		cells.emplace_back(p.username);
		cells.emplace_back(p.password);
		return cells;
	};

	lsbox.at(0).shared_model<std::recursive_mutex>(datas, value_translator, cell_translator);
	button add{ fm, "Add" };
	button load{ fm, "load" };
	button key{ fm, "set key" };
	button save{ fm, "save" };

	load.events().click([&]{
		#ifdef _WIN32
				std::string cmd = std::string("start ")+std::string(argv[0]);
				system(cmd.c_str());
				std::quick_exit(0);
		#elif __APPLE__
					std::string cmd = "open " + resourcePath() + "../../../" + std::string(argv[0]);
					system(cmd.c_str());
					std::quick_exit(0);
		#else
				std::string cmd = std::string("./") + std::string(argv[0]);
				system(cmd.c_str());
				std::quick_exit(0);
		#endif
	});

	save.events().click([&] {
		for (int i = 0; i < inline_widget2::checkboxes.size(); i++) {
			if (inline_widget2::checkboxes[i]->checked()) {
				msgbox mb(fm, "Error");
				mb << L"All boxes must be locked before saving";
				mb.show();
				return false;
			}
		}
		char * lpFileName = (char*)fname.c_str();
		char * tmpo = new char[datas.size() * 100];
		std::stringstream tmp;
		tmp << (int)datas.size() * 2;
		char c = 7;
		for (auto a : datas) tmp << a.username << c << a.password.c_str() << c;
		auto writeBuffer = tmp.str();
		int dataSize = tmp.str().size() * sizeof(char);
		for (int i = 0; i < dataSize; i++) {
			tmpo[i] = writeBuffer[i];
		}
		__asm {
			movzx ebx, keypass;
			rol bl, 4;
			mov ecx, dataSize;
			mov esi, tmpo;
			mov edi, tmpo;
		encrypt_loop:
			lodsb;
			xor al, bl;
			stosb;
			loop encrypt_loop;
		}
		__asm {
			xor eax, eax
			push eax; 7 param - Handle
			push 128; 6 param - /*FILE_ATTRIBUTE_NORMAL*/
			push 2; 5 param - /*OPEN_ALWAYS*/
			push eax; 4 param - Default security
			push 0x00000002; 3 param - share write
			push 0x10000000; 2 param - Generic All access
			push lpFileName; 1 param - FileName
			call CreateFileA;
			pushad
			call GetLastError;

			popad
			pushad;
			mov ecx, dataSize;
			push 0; LPOVERLAPPED
			push 0; /*NULL if */an asynchronous operation
			push ecx; maximum number of bytes to be write
			push tmpo; buffer to write
			push eax; hfile
			call WriteFile;
			popad;
			push eax;
			call CloseHandle;
		}
	});

	add.events().click([&] {//&fm,&datas,&lsbox
		std::string username;
		std::string password;
		inputbox::text user("Username");
		inputbox::text newkey("Password");
		inputbox passkey(fm, "Enter your new username and Password ", "New Entry");
		passkey.verify([&newkey,&user](nana::window handle) {;
		if (newkey.value().empty()) {
			msgbox mb(handle, "Invalid Input");
			mb << L"Passkey is required";
			mb.show();
			return false;
		}
		if (user.value().empty()) {
			msgbox mb(handle, "Invalid Input");
			mb << L"Passkey is required";
			mb.show();
			return false;
		}
		return true;
		});

		if (passkey.show(user, newkey)) {
			username = user.value();
			password = newkey.value();
		}
		if (!username.empty() || !password.empty()) {
			lsbox.at(0).append({ username,password });
			lsbox.at(0).back().check(1);
		}
		//auto mdg = lsbox.at(0).model();
		//datas.push_back(data{ "Who", "10000" });
		//std::cout<<datas.size();
		nana::API::refresh_window(lsbox);
		//fm.close();
	});

	key.events().click([&] {
		inputbox::text pass("Passkey");
		inputbox passkey(fm, "Please enter a number as your key", "Set pass key");
		passkey.verify([&pass](nana::window handle) {;
			for (int i = 0; i < inline_widget2::checkboxes.size(); i++) {
				if (!inline_widget2::checkboxes[i]->checked()) {
					msgbox mb(handle, "Error");
					mb << L"All boxes must be unlocked before modifing passkey";
					mb.show();
					return false;
				}
			}
			if (pass.value().empty()) {
				msgbox mb(handle, "Invalid Input");
				mb << L"Passkey is required";
				mb.show();
				return false;
			}
			for (char a : pass.value()) {
				if (a < '0' || a > '9') {
					msgbox mb(handle, "Invalid Input");
					mb << L"Passkey must contain integer only";
					mb.show();
					return false;
				}
			}
			return true;
		});

		if (passkey.show(pass)) {
			keypass = std::stoi(pass.value()) % 7 + 1;
		}
	});



	//Set the inline_widget, the first column of category 0, the second column of category 1
	lsbox.at(0).inline_factory(0, pat::make_factory<inline_widget>());
	lsbox.at(0).inline_factory(1, pat::make_factory<inline_widget2>());
	//Layout management
	//fm.div("vert <><<><weight=80% text><>><><weight=24<><button><>><>");
	fm.div("< <vert <weight=80%<weight=80% text>| <vert <load><add><key><save> >> |<button>> > ");
	fm["text"] << lsbox;
	fm["button"] << qut;
	fm["add"] << add;
	fm["load"] << load;
	fm["save"] << save;
	fm["key"] << key;
	fm.collocate();

	fm.show();
	nana::exec();
	return 0;
}

