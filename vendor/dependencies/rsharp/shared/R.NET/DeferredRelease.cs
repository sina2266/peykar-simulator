using System;
using System.Collections.Concurrent;

namespace RDotNet
{
   /// <summary>
   ///    Deferred, R-main-thread release of preserved SEXP handles.
   ///    <para>
   ///    When a <see cref="SymbolicExpression" /> SafeHandle finalizes on the CLR finalizer
   ///    thread it cannot call R_ReleaseObject there: R's memory manager is single threaded
   ///    and non-reentrant, so releasing off the R main thread corrupts R's heap (a Windows
   ///    access violation or a Linux hang). Instead the finalizer queues the handle here and
   ///    the R main thread releases it later via <see cref="Drain" />, called from the
   ///    marshalling layer at a known-safe point. Queuing rather than skipping the release
   ///    means the preservation is not leaked — the object is still released, just on the
   ///    safe thread.
   ///    </para>
   /// </summary>
   public static class DeferredRelease
   {
      private static readonly ConcurrentQueue<IntPtr> _pending = new ConcurrentQueue<IntPtr>();

      /// <summary>
      ///    The release mechanism (R_ReleaseObject, taking the engine's lock when enabled),
      ///    supplied once by <see cref="SymbolicExpression" /> on the R main thread. Held here
      ///    so the static <see cref="Drain" /> needs no other access to engine internals.
      /// </summary>
      private static Action<IntPtr> _release;

      /// <summary>
      ///    Supplies the release mechanism. Idempotent; the first (R main thread) call wins.
      /// </summary>
      internal static void Configure(Action<IntPtr> release)
      {
         if (_release == null)
            _release = release;
      }

      /// <summary>
      ///    Queue a handle whose SafeHandle finalized off the R main thread, for the R main
      ///    thread to release later.
      /// </summary>
      internal static void Enqueue(IntPtr handle)
      {
         _pending.Enqueue(handle);
      }

      /// <summary>
      ///    Release all queued handles. Must be called only from the R main thread (e.g. during
      ///    a marshalling call). A no-op when nothing is queued, or before the release
      ///    mechanism has been supplied.
      /// </summary>
      public static void Drain()
      {
         if (_release == null || _pending.IsEmpty)
            return;

         while (_pending.TryDequeue(out var handle))
         {
            try
            {
               _release(handle);
            }
            catch
            {
               /* never let cleanup throw across the native boundary */
            }
         }
      }
   }
}
