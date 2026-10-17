import { createContext, useContext } from 'react';
import { Client, type Ref, type RunKind } from './api';
export const SessionContext = createContext<
  | {
      client: Client;
      admin: boolean;
      openRun: (kind: RunKind, id: string) => void;
      createRun: (kind: 'reference' | 'native', model: Ref) => void;
    }
  | undefined
>(undefined);
export function useSession() {
  const value = useContext(SessionContext);
  if (!value) throw new Error('Session unavailable');
  return value;
}
