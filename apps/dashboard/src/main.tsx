import { StrictMode, useEffect, useMemo, useState, Component, type ReactNode } from 'react';
import { createRoot } from 'react-dom/client';
import { Client, runKinds, type Ref, type RunKind } from './api';
import { LocaleContext, useT, type Locale } from './i18n';
import { SessionContext } from './session';
import { Models, Biology, Datasets } from './catalogs';
import { Experiments } from './experiments';
import { Viewer } from './viewer';
import { Validation } from './validation';
import { ErrorBox, Field, Icon, useResource } from './ui';
import './style.css';

const pages = [
  {
    key: 'biology',
    name: 'Biological Explorer',
    icon: 'biology',
    group: 'Explore',
  },
  { key: 'models', name: 'Model Studio', icon: 'layers', group: 'Explore' },
  { key: 'datasets', name: 'Dataset Studio', icon: 'data', group: 'Explore' },
  {
    key: 'experiments',
    name: 'Experiment Studio',
    icon: 'experiment',
    group: 'Build & run',
  },
  {
    key: 'viewer',
    name: 'Simulation Viewer',
    icon: 'chart',
    group: 'Review',
  },
  {
    key: 'validation',
    name: 'Validation Center',
    icon: 'check',
    group: 'Review',
  },
];
function readRoute() {
  const [page = 'experiments', kind, id] = location.hash.slice(1).replace(/^\//, '').split('/');
  return {
    page: pages.some((p) => p.key === page) ? page : 'experiments',
    kind: runKinds.includes(kind as RunKind) ? (kind as RunKind) : undefined,
    id: id ? decodeRouteId(id) : undefined,
  };
}
function decodeRouteId(id: string) {
  try {
    return decodeURIComponent(id);
  } catch {
    return undefined;
  }
}
function App() {
  const [locale, setLocale] = useState<Locale>('en');
  const [token, setToken] = useState('');
  const [role, setRole] = useState('');
  const [expired, setExpired] = useState(false);
  useEffect(() => {
    document.documentElement.lang = locale;
    document.documentElement.dir = locale === 'fa' ? 'rtl' : 'ltr';
  }, [locale]);
  const client = useMemo(
    () =>
      new Client(token, () => {
        setToken((current) => (current === token ? '' : current));
        setExpired(true);
      }),
    [token],
  );
  return (
    <LocaleContext value={locale}>
      <div className="app">
        <button
          className="language"
          aria-label="Language / زبان"
          onClick={() => setLocale((l) => (l === 'en' ? 'fa' : 'en'))}
        >
          {locale === 'en' ? 'فارسی' : 'English'}
        </button>
        {token ? (
          <Workspace
            client={client}
            role={role}
            disconnect={() => {
              setToken('');
              setRole('');
              setExpired(false);
            }}
          />
        ) : (
          <Login
            expired={expired}
            connected={(key, access) => {
              setToken(key);
              setRole(access);
              setExpired(false);
            }}
          />
        )}
      </div>
    </LocaleContext>
  );
}
function Brand() {
  return (
    <span className="brand">
      <svg viewBox="0 0 40 40" fill="none" aria-hidden="true">
        <path d="M10 13 21 6l11 8v13L21 34 10 27V13Z" />
        <path d="m10 13 11 7 11-6M21 20v14M5 9v20" />
        <circle cx="21" cy="6" r="3" />
        <circle cx="32" cy="27" r="3" />
        <circle cx="10" cy="27" r="3" />
      </svg>
      <span>
        Peykar<span className="brand-dot">.</span>
      </span>
    </span>
  );
}
function Login({
  connected,
  expired,
}: {
  connected: (token: string, role: string) => void;
  expired: boolean;
}) {
  const t = useT();
  const [value, setValue] = useState('');
  const [error, setError] = useState<unknown>();
  const [busy, setBusy] = useState(false);
  return (
    <main className="login">
      <section className="login-story">
        <Brand />
        <p className="eyebrow">BIOLOGICAL MODELING & SIMULATION</p>
        <h1>{t('From biological context to traceable computation.')}</h1>
        <p>{t('Inspect evidence. Freeze an experiment. Review what actually ran.')}</p>
        <div className="brand-orbit" aria-hidden="true">
          <svg viewBox="0 0 500 240">
            <ellipse cx="250" cy="120" rx="190" ry="65" />
            <ellipse cx="250" cy="120" rx="190" ry="65" transform="rotate(28 250 120)" />
            <ellipse cx="250" cy="120" rx="190" ry="65" transform="rotate(-28 250 120)" />
            <circle cx="250" cy="120" r="35" />
            <circle cx="95" cy="82" r="9" />
            <circle cx="416" cy="145" r="7" />
          </svg>
        </div>
        <span className="login-footer">{t('Research use only')} · v0.3.0</span>
      </section>
      <section className="login-form">
        <p className="eyebrow">{t('Local workspace')}</p>
        <h2>{t('Connect to your workspace')}</h2>
        <p>{t('Use the administrator or reader token configured for this API.')}</p>
        <form
          className="stack"
          onSubmit={async (e) => {
            e.preventDefault();
            setBusy(true);
            setError(undefined);
            try {
              const result = await new Client(value).request<{
                role: string;
              }>('/v1/workspace/session');
              connected(value, result.role);
              setValue('');
            } catch (err) {
              setError(err);
            } finally {
              setBusy(false);
            }
          }}
        >
          <Field label="Access token">
            <input
              autoComplete="off"
              type="password"
              required
              minLength={24}
              value={value}
              onChange={(e) => setValue(e.target.value)}
              autoFocus
            />
          </Field>
          <p className="muted">
            {t('The token stays in this page’s memory. Reloading or disconnecting clears it.')}
          </p>
          {expired && (
            <div className="notice" role="status">
              Session expired. Connect with a valid token.
            </div>
          )}
          {error !== undefined && <ErrorBox error={error} />}
          <button className="primary" disabled={busy}>
            {t(busy ? 'Connecting…' : 'Connect')} <Icon name="arrow" />
          </button>
        </form>
      </section>
    </main>
  );
}
function Workspace({
  client,
  role,
  disconnect,
}: {
  client: Client;
  role: string;
  disconnect: () => void;
}) {
  const t = useT();
  const [route, setRoute] = useState(readRoute);
  const [menu, setMenu] = useState(false);
  const [initial, setInitial] = useState<{
    kind: 'reference' | 'native';
    model: Ref;
  }>();
  const [draft, setDraft] = useState(0);
  useEffect(() => {
    const changed = () => {
      setRoute(readRoute());
      setMenu(false);
    };
    window.addEventListener('hashchange', changed);
    return () => window.removeEventListener('hashchange', changed);
  }, []);
  const health = useResource(
    (s) => client.request('/v1/workspace/session', { signal: s }),
    [client],
    15000,
  );
  useEffect(() => {
    const escape = (e: KeyboardEvent) => {
      if (e.key === 'Escape' && menu) {
        setMenu(false);
        document.querySelector<HTMLButtonElement>('.menu-button')?.focus();
      }
    };
    window.addEventListener('keydown', escape);
    return () => window.removeEventListener('keydown', escape);
  }, [menu]);
  const page = pages.find((p) => p.key === route.page)!;
  const openRun = (kind: RunKind, id: string) => {
    location.hash = `/viewer/${kind}/${encodeURIComponent(id)}`;
  };
  const createRun = (kind: 'reference' | 'native', model: Ref) => {
    setInitial({ kind, model });
    setDraft((d) => d + 1);
    location.hash = '/experiments';
  };
  return (
    <SessionContext value={{ client, admin: role === 'admin', openRun, createRun }}>
      <a
        className="skip-link"
        href="#main"
        onClick={(e) => {
          e.preventDefault();
          document.getElementById('main')?.focus();
        }}
      >
        {t('Skip to content')}
      </a>
      <aside id="workspace-navigation" className={`sidebar ${menu ? 'open' : ''}`}>
        <a className="brand-link" href="#/experiments" aria-label="Peykar">
          <Brand />
        </a>
        <p className="workspace-label">{t('Research workbench')}</p>
        <nav aria-label={t('Research workbench')}>
          {['Explore', 'Build & run', 'Review'].map((group) => (
            <div className="nav-group" key={group}>
              <p>{t(group)}</p>
              {pages
                .filter((p) => p.group === group)
                .map((p) => (
                  <a
                    key={p.key}
                    href={`#/${p.key}`}
                    aria-current={route.page === p.key ? 'page' : undefined}
                    onClick={() => setMenu(false)}
                  >
                    <Icon name={p.icon} />
                    <span>{t(p.name)}</span>
                  </a>
                ))}
            </div>
          ))}
        </nav>
        <footer>
          <span className="local-dot" />
          {t('Local workspace')}
          <p>{t('Research use only')}</p>
        </footer>
      </aside>
      <div className="main-shell">
        <header className="topbar">
          <button
            className="menu-button"
            aria-controls="workspace-navigation"
            onClick={() => setMenu((v) => !v)}
            aria-expanded={menu}
          >
            {t('Menu')}
          </button>
          <span className="breadcrumb">
            Peykar <span>/</span> {t(page.name)}
          </span>
          <div className="session-tools">
            <span className="connection">
              <span />
              {t(health.error ? 'Service unavailable' : 'Connected')}
            </span>
            <span className="role">{t(role === 'admin' ? 'Administrator' : 'Read only')}</span>
            <button onClick={disconnect}>{t('Disconnect')}</button>
          </div>
        </header>
        <main id="main" tabIndex={-1} className="workspace-main">
          <RenderBoundary key={`${route.page}:${route.id || ''}`}>
            {route.page === 'biology' ? (
              <Biology />
            ) : route.page === 'models' ? (
              <Models />
            ) : route.page === 'datasets' ? (
              <Datasets />
            ) : route.page === 'experiments' ? (
              <Experiments key={draft} initial={initial} />
            ) : route.page === 'viewer' ? (
              <Viewer
                key={`${route.kind}:${route.id}`}
                selectedKind={route.kind}
                selectedId={route.id}
              />
            ) : (
              <Validation />
            )}
          </RenderBoundary>
        </main>
        <footer className="main-footer">
          <span>Peykar / 0.3.0</span>
          <span>{t('Research use only')} · Scientific validity is context specific</span>
        </footer>
      </div>
    </SessionContext>
  );
}
class RenderBoundary extends Component<{ children: ReactNode }, { error?: Error }> {
  state: { error?: Error } = {};
  static getDerivedStateFromError(error: Error) {
    return { error };
  }
  render() {
    return this.state.error ? (
      <ErrorBox
        error={
          new Error(
            'This view could not be displayed. Reload the page to reconnect; saved server records remain available.',
          )
        }
      />
    ) : (
      this.props.children
    );
  }
}
createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <App />
  </StrictMode>,
);
