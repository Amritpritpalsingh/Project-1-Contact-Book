#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

struct Contact {
    char name[50];
    char phone[20];
    char email[50];
    Contact* next;
};

struct FileContact {
    char name[50];
    char phone[20];
    char email[50];
};

const char* DATA_FILE = "contacts.dat";

void trim_newline(char *s){
    size_t n = strlen(s);
    while(n>0 && (s[n-1]=='\n' || s[n-1]=='\r' || s[n-1]==' ')) s[--n] = '\0';
}

void input_line(const char* prompt, char* buf, size_t maxlen){
    cout << prompt;
    cin.getline(buf, (int)maxlen);
    buf[maxlen-1] = '\0';
    trim_newline(buf);
}

int name_cmp(const char* a, const char* b){
    // case-insensitive compare (simple)
    for(; *a && *b; ++a, ++b){
        char ca = (*a>='A' && *a<='Z') ? *a - 'A' + 'a' : *a;
        char cb = (*b>='A' && *b<='Z') ? *b - 'A' + 'a' : *b;
        if(ca != cb) return (ca<cb) ? -1 : 1;
    }
    if(*a==*b) return 0;
    return *a ? 1 : -1;
}

void insert_sorted(Contact*& head, const char* name, const char* phone, const char* email){
    Contact* node = new Contact;
    strncpy(node->name, name, sizeof(node->name)); node->name[sizeof(node->name)-1] = '\0';
    strncpy(node->phone, phone, sizeof(node->phone)); node->phone[sizeof(node->phone)-1] = '\0';
    strncpy(node->email, email, sizeof(node->email)); node->email[sizeof(node->email)-1] = '\0';
    node->next = nullptr;

    if(!head || name_cmp(name, head->name) < 0){
        node->next = head;
        head = node;
        return;
    }
    Contact* cur = head;
    while(cur->next && name_cmp(cur->next->name, name) <= 0) cur = cur->next;
    node->next = cur->next;
    cur->next = node;
}

Contact* search(Contact* head, const char* name, Contact** prev_out){
    Contact* prev = nullptr;
    Contact* cur = head;
    while(cur){
        if(name_cmp(cur->name, name) == 0){
            if(prev_out) *prev_out = prev;
            return cur;
        }
        prev = cur;
        cur = cur->next;
    }
    if(prev_out) *prev_out = nullptr;
    return nullptr;
}

bool remove_contact(Contact*& head, const char* name){
    Contact* prev = nullptr;
    Contact* cur = search(head, name, &prev);
    if(!cur) return false;
    if(prev) prev->next = cur->next;
    else head = cur->next;
    delete cur;
    return true;
}

void display_all(Contact* head){
    if(!head){ cout << "No contacts.\n"; return; }
    int i=1;
    for(Contact* c=head; c; c=c->next, ++i){
        cout << i << ". " << c->name << " | " << c->phone << " | " << c->email << "\n";
    }
}

void update_contact(Contact* head){
    char key[50]; input_line("Enter name to update: ", key, sizeof(key));
    Contact* c = search(head, key, nullptr);
    if(!c){ cout << "Contact not found.\n"; return; }
    char buf[100];
    input_line("New phone (leave empty to keep): ", buf, sizeof(buf));
    if(strlen(buf)) { strncpy(c->phone, buf, sizeof(c->phone)); c->phone[sizeof(c->phone)-1]='\0'; }
    input_line("New email (leave empty to keep): ", buf, sizeof(buf));
    if(strlen(buf)) { strncpy(c->email, buf, sizeof(c->email)); c->email[sizeof(c->email)-1]='\0'; }
    cout << "Updated.\n";
}

void save_to_file(Contact* head){
    FILE* fp = fopen(DATA_FILE, "wb");
    if(!fp){ cout << "Error opening file for write.\n"; return; }
    for(Contact* c=head; c; c=c->next){
        FileContact fc;
        strncpy(fc.name, c->name, sizeof(fc.name));
        strncpy(fc.phone, c->phone, sizeof(fc.phone));
        strncpy(fc.email, c->email, sizeof(fc.email));
        fc.name[sizeof(fc.name)-1]='\0';
        fc.phone[sizeof(fc.phone)-1]='\0';
        fc.email[sizeof(fc.email)-1]='\0';
        fwrite(&fc, sizeof(fc), 1, fp);
    }
    fclose(fp);
}

void load_from_file(Contact*& head){
    head = nullptr;
    FILE* fp = fopen(DATA_FILE, "rb");
    if(!fp) return; // no file yet
    FileContact fc;
    while(fread(&fc, sizeof(fc), 1, fp) == 1){
        insert_sorted(head, fc.name, fc.phone, fc.email);
    }
    fclose(fp);
}

void free_all(Contact*& head){
    while(head){ Contact* t=head; head=head->next; delete t; }
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Contact* head = nullptr;
    load_from_file(head);

    while(true){
        cout << "\n=== Contact Book ===\n"
             << "1. Add Contact\n2. Search Contact\n3. Update Contact\n4. Delete Contact\n5. View All\n6. Save\n7. Exit\nChoice: ";
        int ch; if(!(cin>>ch)){ return 0; }
        cin.ignore(1024, '\n');
        if(ch==1){
            char name[50], phone[20], email[50];
            input_line("Name: ", name, sizeof(name));
            input_line("Phone: ", phone, sizeof(phone));
            input_line("Email: ", email, sizeof(email));
            insert_sorted(head, name, phone, email);
            cout << "Added.\n";
        } else if(ch==2){
            char key[50]; input_line("Enter name to search: ", key, sizeof(key));
            Contact* c = search(head, key, nullptr);
            if(c) cout << c->name << " | " << c->phone << " | " << c->email << "\n";
            else cout << "Not found.\n";
        } else if(ch==3){
            update_contact(head);
        } else if(ch==4){
            char key[50]; input_line("Enter name to delete: ", key, sizeof(key));
            cout << (remove_contact(head,key) ? "Deleted.\n" : "Not found.\n");
        } else if(ch==5){
            display_all(head);
        } else if(ch==6){
            save_to_file(head);
            cout << "Saved to " << DATA_FILE << ".\n";
        } else if(ch==7){
            save_to_file(head);
            free_all(head);
            cout << "Goodbye!\n";
            break;
        } else {
            cout << "Invalid choice.\n";
        }
    }
    return 0;
}
