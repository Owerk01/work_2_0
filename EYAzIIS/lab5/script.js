document.addEventListener('DOMContentLoaded', () => {
  const chatWindow = document.getElementById('chat-window');
  const userInput = document.getElementById('user-input');
  const sendBtn = document.getElementById('send-btn');
  const deleteBtn = document.getElementById('delete-chat-btn'); 
  const helpPanel = document.getElementById('help-panel');
  const toggleHelpBtn = document.getElementById('toggle-help');
  const closeHelpBtn = document.getElementById('close-help');
  const currentUserEl = document.getElementById('current-user');

  
  let USER_ID = localStorage.getItem('hobby_user_id');
  if (!USER_ID) {
    USER_ID = Math.floor(Math.random() * 1e9).toString();
    localStorage.setItem('hobby_user_id', USER_ID);
  }
  if (currentUserEl) currentUserEl.textContent = USER_ID;

  function addMessage(text, sender) {
    const msg = document.createElement('div');
    msg.classList.add('message', sender);
    msg.innerHTML = `<div class="message-content">${escapeHtml(text)}</div>`;
    chatWindow.appendChild(msg);
    scrollToBottom();
  }

  function escapeHtml(str) {
    const div = document.createElement('div');
    div.textContent = str;
    return div.innerHTML;
  }

  function scrollToBottom() {
    chatWindow.scrollTop = chatWindow.scrollHeight;
  }

  
  async function loadChat() {
    try {
      const res = await fetch(`/api/chat/${USER_ID}`);
      if (res.ok) {
        const messages = await res.json();
        chatWindow.innerHTML = '';
        messages.forEach(msg => addMessage(msg.text, msg.sender));
      }
    } catch (e) {
      console.warn('Could not load chat:', e);
    }
  }

  async function handleSend() {
    const text = userInput.value.trim();
    if (!text) return;

    addMessage(text, 'user');
    userInput.value = '';
    userInput.style.height = 'auto';

    
    const typing = document.createElement('div');
    typing.className = 'message system';
    typing.innerHTML = `<div class="message-content">Thinking...</div>`;
    chatWindow.appendChild(typing);
    scrollToBottom();

    try {
      const res = await fetch('/api/chat', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ user_id: USER_ID, query: text })
      });

      typing.remove();

      if (!res.ok) {
        const err = await res.json().catch(() => ({}));
        throw new Error(err.error || 'API error');
      }

      const data = await res.json();
      addMessage(data.response, 'system');
    } catch (error) {
      typing.remove();
      addMessage(`Error: ${error.message}`, 'system');
      console.error('Chat error:', error);
    }
  }

  async function handleDeleteChat() {
    if (!confirm('Delete entire chat history?')) return;
    try {
      const res = await fetch(`/api/chat/${USER_ID}`, { method: 'DELETE' });
      if (res.ok) {
        chatWindow.innerHTML = '<div class="message system"><div class="message-content"></div></div>';
      }
    } catch (e) {
      alert('Failed to delete: ' + e.message);
    }
  }

  
  sendBtn.addEventListener('click', handleSend);
  if (deleteBtn) deleteBtn.addEventListener('click', handleDeleteChat);

  userInput.addEventListener('keydown', (e) => {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      handleSend();
    }
  });

  userInput.addEventListener('input', function() {
    this.style.height = 'auto';
    if (this.scrollHeight <= 120) this.style.height = this.scrollHeight + 'px';
    else this.style.height = '120px';
  });

  if (toggleHelpBtn) {
    toggleHelpBtn.addEventListener('click', (e) => {
      e.stopPropagation();
      helpPanel.classList.add('open');
    });
  }
  if (closeHelpBtn) {
    closeHelpBtn.addEventListener('click', () => helpPanel.classList.remove('open'));
  }

  loadChat();
});