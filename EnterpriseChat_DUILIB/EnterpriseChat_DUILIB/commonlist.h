#ifndef _COMMONLIST_H_
#define _COMMONLIST_H_
#include <UIlib.h>
#include <list>
#include "CommonData.h"

typedef struct _NodeData
{
	int  m_level;
	bool m_folder;
	bool m_child_visible;
	bool m_has_child;
	DuiLib::CDuiString m_text;
	DuiLib::CDuiString m_value;
	DuiLib::CListContainerElementUI* m_list_elment;
}NodeData;


class Node
{
public:
	Node();
	explicit Node(NodeData t);
	Node(NodeData t, Node* parent);
	~Node();
	NodeData& data();
	int num_children() const;
	Node* child(int i);
	Node* parent();
	bool folder() const;
	bool has_children() const;
	void add_child(Node* child);
	void remove_child(Node* child);
	Node* get_last_child();
private:
	void set_parent(Node* parent);
private:
	std::list<Node*>	m_listChildren;
	Node*		m_parent;
	NodeData    m_data;
};

class commonlist :
	public DuiLib::CListUI
{
public:
	commonlist(DuiLib::CPaintManagerUI& paint_manager);

	~commonlist(void);

	bool Add(CControlUI* pControl);

	bool AddAt(CControlUI* pControl, int iIndex);

	bool Remove(CControlUI* pControl);

	bool RemoveAt(int iIndex);

	void RemoveAll();

	void DoEvent(DuiLib::TEventUI& event);

	Node* GetRoot();

	Node* AddNode(const FriendListItemInfo& item, Node* parent = NULL);

	bool RemoveNode(Node* node);

	void SetChildVisible(Node* node, bool visible);

	bool CanExpand(Node* node) const;

	bool SelectItem(int iIndex, bool bTakeFocus = false);
private:
	Node*	m_root_node;
	LONG	m_delay_deltaY;
	DWORD	m_delay_number;
	DWORD	m_delay_left;
	DuiLib::CDuiRect	m_text_padding;
	int		m_level_text_start_pos;
	DuiLib::CDuiString			m_level_expand_image;
	DuiLib::CDuiString			m_level_collapse_image;
	DuiLib::CPaintManagerUI&	m_paint_manager;
    DuiLib::CDialogBuilder		m_dlgBuilder;
};
#endif