#include "CommonData.h"
#include "commonlist.h"


const TCHAR* const kLogoButtonControlName = _T("logo");
const TCHAR* const kLogoContainerControlName = _T("logo_container");
const TCHAR* const kNickNameControlName = _T("nickname");
const TCHAR* const kDescriptionControlName = _T("description");
const TCHAR* const kOperatorPannelControlName = _T("operation";)
 
const int kFriendListItemNormalHeight = 32;
const int kFriendListItemSelectedHeight = 50;

using namespace DuiLib;

//Node¿‡
Node::Node()
	:m_parent(NULL)
{
	m_listChildren.clear();
}

Node::Node(NodeData t)
	:m_parent(NULL),
	m_data(t)
{
	m_listChildren.clear();
}

Node::Node(NodeData t, Node* parent)
	:m_data(t),
	m_parent(parent)
{
	m_listChildren.clear();
}

Node::~Node()
{
	Node* pNode=NULL;
	for(list<Node*>::iterator ite=m_listChildren.begin();ite!=m_listChildren.end();)
	{
		pNode=*ite;
		RELEASE(pNode);
		ite=m_listChildren.begin();
	}
}

NodeData& Node::data()
{
	return m_data;
}

int Node::num_children() const
{
	return m_listChildren.size();
}

Node* Node::child(int i)
{
	Node* pNode=NULL;
	if(i>=(int)(m_listChildren.size())||i<0)
	{
		return pNode;
	}
	list<Node*>::iterator ite=m_listChildren.begin();
	for(int index=0;index<=i;++index)
	{
		pNode=*ite;
		++ite;
	}
	return pNode;
}

Node* Node::parent()
{
	return m_parent;
}

bool Node::folder() const
{
	return m_data.m_folder;
}

bool Node::has_children() const
{
	return m_listChildren.size()>0;
}

void Node::add_child(Node* child)
{
	child->set_parent(this);
	m_listChildren.push_back(child);
}

void Node::remove_child(Node* child)
{
	m_listChildren.remove(child);
}

Node* Node::get_last_child()
{
	if (has_children())
	{
		return child(num_children() - 1)->get_last_child();
	}
	return this;
}

void Node::set_parent(Node* parent)
{
	m_parent=parent;
}

//commonlist¿‡
commonlist::commonlist(CPaintManagerUI& paint_manager)
	: m_root_node(NULL)
	, m_delay_deltaY(0)
	, m_delay_number(0)
	, m_delay_left(0)
	, m_level_expand_image(_T("<i list_icon_b.png>"))
	, m_level_collapse_image(_T("<i list_icon_a.png>"))
	, m_level_text_start_pos(10)
	, m_text_padding(10, 0, 0, 0)
	, m_paint_manager(paint_manager)
{
	SetItemShowHtml(true);
	m_root_node = new Node;
	m_root_node->data().m_level = -1;
	m_root_node->data().m_child_visible = true;
	m_root_node->data().m_has_child= true;
	m_root_node->data().m_list_elment = NULL;
}

commonlist::~commonlist(void)
{
	RELEASE(m_root_node);
}

bool commonlist::Add(CControlUI* pControl)
{
	if (!pControl)
	{
		return false;
	}
	if (_tcsicmp(pControl->GetClass(), _T("ListContainerElementUI")) != 0)
	{
		return false;
	}

	return CListUI::Add(pControl);
}

bool commonlist::AddAt(CControlUI* pControl, int iIndex)
{
	if (!pControl)
		return false;

	if (_tcsicmp(pControl->GetClass(), _T("ListContainerElementUI")) != 0)
		return false;

	return CListUI::AddAt(pControl, iIndex);
}

bool commonlist::Remove(CControlUI* pControl)
{
	if (!pControl)
	{
		return false;
	}

	if (_tcsicmp(pControl->GetClass(), _T("ListContainerElementUI")) != 0)
	{
		return false;
	}

	if (reinterpret_cast<Node*>(static_cast<CListContainerElementUI*>(pControl->GetInterface(_T("ListContainerElement")))->GetTag()) == NULL)
	{
		return CListUI::Remove(pControl);
	}
	else
	{
		return RemoveNode(reinterpret_cast<Node*>(static_cast<CListContainerElementUI*>(pControl->GetInterface(_T("ListContainerElement")))->GetTag()));
	}
}

bool commonlist::RemoveAt(int iIndex)
{
	CControlUI* pControl = GetItemAt(iIndex);
	if (!pControl)
	{
		return false;
	}
	if (_tcsicmp(pControl->GetClass(), _T("ListContainerElementUI")) != 0)
	{
		return false;
	}
	if (reinterpret_cast<Node*>(static_cast<CListContainerElementUI*>(pControl->GetInterface(_T("ListContainerElement")))->GetTag()) == NULL)
	{
		return CListUI::RemoveAt(iIndex);
	}
	else
	{
		return RemoveNode(reinterpret_cast<Node*>(static_cast<CListContainerElementUI*>(pControl->GetInterface(_T("ListContainerElement")))->GetTag()));
	}
}

void commonlist::RemoveAll()
{
	CListUI::RemoveAll();
	for (int i = 0; i < m_root_node->num_children(); ++i)
	{
		Node* child = m_root_node->child(i);
		RemoveNode(child);
	}
	RELEASE(m_root_node);

	m_root_node = new Node;
	m_root_node->data().m_level = -1;
	m_root_node->data().m_child_visible = true;
	m_root_node->data().m_has_child = true;
	m_root_node->data().m_list_elment = NULL;
}

void commonlist::DoEvent(DuiLib::TEventUI& event)
{
	CListUI::DoEvent(event);
}

Node* commonlist::GetRoot()
{
	return m_root_node;
}

Node* commonlist::AddNode(const FriendListItemInfo& item, Node* parent )
{
	if (!parent)
	{
		parent = m_root_node;
	}

	TCHAR szBuf[MAX_PATH] = {0};

	CListContainerElementUI* pListElement = NULL;
	if( !m_dlgBuilder.GetMarkup()->IsValid() ) 
	{
		pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create(_T("friend_list_item.xml"), (UINT)0, NULL, &m_paint_manager));
	}
	else 
	{
		pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create((UINT)0, &m_paint_manager));
	}
	if (pListElement == NULL)
	{
		return NULL;
	}
	Node* node = new Node;

	node->data().m_level = parent->data().m_level + 1;
	if (item.m_folder)
	{
		node->data().m_has_child= !item.m_empty;
	}
	else
	{
		node->data().m_has_child = false;
	}

	node->data().m_folder = item.m_folder;

	node->data().m_child_visible = (node->data().m_level == 0);
	node->data().m_child_visible = false;

	node->data().m_text = item.m_nick_name;
	node->data().m_value = item.m_id;
	node->data().m_list_elment= pListElement;

	if (!parent->data().m_child_visible)
	{
		pListElement->SetVisible(false);
	}
	if (parent != m_root_node && !parent->data().m_list_elment->IsVisible())
	{
		pListElement->SetVisible(false);
	}
	CDuiRect rcPadding =m_text_padding;
	for (int i = 0; i < node->data().m_level; ++i)
	{
		rcPadding.left += m_level_text_start_pos;		
	}
	pListElement->SetPadding(rcPadding);

	CButtonUI* log_button = static_cast<CButtonUI*>(m_paint_manager.FindSubControlByName(pListElement, kLogoButtonControlName));
	if (log_button != NULL)
	{
		if (!item.m_folder && !item.m_logo.IsEmpty())
		{
			_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), item.m_logo);
			log_button->SetNormalImage(szBuf);
		}
		else
		{
			CContainerUI* logo_container = static_cast<CContainerUI*>(m_paint_manager.FindSubControlByName(pListElement, kLogoContainerControlName));
			if (logo_container != NULL)
			{
				logo_container->SetVisible(false);
			}
		}
		log_button->SetTag((UINT_PTR)pListElement);
		//log_button->OnEvent += MakeDelegate(&OnLogoButtonEvent);
	}

	CDuiString html_text;
	if (node->data().m_has_child)
	{
		if (node->data().m_child_visible)
		{
			html_text += m_level_expand_image;
		}
		else
		{
			html_text += m_level_collapse_image;
		}
		_stprintf_s(szBuf, MAX_PATH - 1, _T("<x %d>"), m_level_text_start_pos);
		html_text += szBuf;
	}

	if (item.m_folder)
	{
		html_text += node->data().m_text;
	}
	else
	{
		_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), item.m_nick_name);
		html_text += szBuf;
	}

	CLabelUI* nick_name = static_cast<CLabelUI*>(m_paint_manager.FindSubControlByName(pListElement, kNickNameControlName));
	if (nick_name != NULL)
	{
		if (item.m_folder)
			nick_name->SetFixedWidth(0);

		nick_name->SetShowHtml(true);
		nick_name->SetText(html_text);
	}

	if (!item.m_folder && !item.m_description.IsEmpty())
	{
		CLabelUI* description = static_cast<CLabelUI*>(m_paint_manager.FindSubControlByName(pListElement, kDescriptionControlName));
		if (description != NULL)
		{
			_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), item.m_description);
			description->SetShowHtml(true);
			description->SetText(szBuf);
		}
	}
	if(node->data().m_folder)
	{
		//pListElement->SetBkColor(0xffffff);//SetAttribute(_T("itemselectedbkcolor"),(LPCTSTR)0xffffff);
	}
	pListElement->SetFixedHeight(kFriendListItemNormalHeight);
	pListElement->SetTag((UINT_PTR)node);
	int index = 0;
	if (parent->has_children())
	{
		Node* prev = parent->get_last_child();
		index = prev->data().m_list_elment->GetIndex() + 1;
	}
	else 
	{
		if (parent == m_root_node)
		{
			index = 0;
		}
		else
		{
			index = parent->data().m_list_elment->GetIndex() + 1;
		}
	}
	if (!CListUI::AddAt(pListElement,index))
	{
		delete pListElement;
		delete node;
		node = NULL;
	}

	parent->add_child(node);
	return node;
}

bool commonlist::RemoveNode(Node* node)
{
	if (!node || node == m_root_node) 
	{
		return false;
	}
	for (int i = 0; i < node->num_children(); ++i)
	{
		Node* child = node->child(i);
		RemoveNode(child);
	}

	CListUI::Remove(node->data().m_list_elment);
	node->parent()->remove_child(node);
	RELEASE(node);

	return true;
}

void commonlist::SetChildVisible(Node* node, bool visible)
{
	if(!node||node==m_root_node)
	{
		return;
	}
	if(node->data().m_child_visible==visible)
	{
		return;
	}
	node->data().m_child_visible=visible;

	TCHAR szBuf[MAX_PATH]={0};
	memset(szBuf,0,sizeof(szBuf));

	CDuiString html_text;
	if(node->data().m_has_child)
	{
		if(node->data().m_child_visible)
		{
			html_text += m_level_expand_image;
		}
		else
		{
			html_text += m_level_collapse_image;
		}
		_stprintf_s(szBuf, MAX_PATH - 1, _T("<x %d>"), m_level_text_start_pos);
		html_text += szBuf;
		html_text += node->data().m_text;

		CLabelUI* nick_name = static_cast<CLabelUI*>(m_paint_manager.FindSubControlByName(node->data().m_list_elment, kNickNameControlName));
		if (nick_name != NULL)
		{
			nick_name->SetShowHtml(true);
			nick_name->SetText(html_text);
		}
	}

	if (!node->data().m_list_elment->IsVisible())
	{
		return;
	}
	if (!node->has_children())
	{
		return;
	}
	Node* begin = node->child(0);
	Node* end = node->get_last_child();
	for (int i = begin->data().m_list_elment->GetIndex(); i <= end->data().m_list_elment->GetIndex(); ++i)
	{
		CControlUI* control = GetItemAt(i);
		if (_tcsicmp(control->GetClass(), _T("ListContainerElementUI")) == 0)
		{
			if (visible) 
			{
				Node* local_parent = ((Node*)control->GetTag())->parent();
				if (local_parent->data().m_child_visible && local_parent->data().m_list_elment->IsVisible())
				{
					control->SetVisible(true);
				}
			}
			else
			{
				control->SetVisible(false);
			}
		}
	}
}

bool commonlist::CanExpand(Node* node) const
{
	if (!node || node == m_root_node)
	{
		return false;
	}
	return node->data().m_has_child;
}

bool commonlist::SelectItem(int iIndex, bool bTakeFocus )
{

	return __super::SelectItem(iIndex,bTakeFocus);
}


