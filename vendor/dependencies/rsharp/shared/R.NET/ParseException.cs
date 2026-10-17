using System;
using System.Runtime.Serialization;
using RDotNet.Internals;

namespace RDotNet
{
   /// <summary>
   ///    Thrown when an engine comes to an error.
   /// </summary>
   [Serializable]
   public class ParseException : Exception
   // (http://msdn.microsoft.com/en-us/library/vstudio/system.applicationexception%28v=vs.110%29.aspx)
   // "If you are designing an application that needs to create its own exceptions,
   // you are advised to derive custom exceptions from the Exception class"
   {
      private const string StatusFieldName = "status";

      private const string ErrorStatementFieldName = "errorStatement";

      /// <summary>
      ///    Creates a new instance.
      /// </summary>
      private ParseException()
         : this(ParseStatus.Null, "", "")
      // This does not internally occur. See Parse.h in R_HOME/include/R_ext/Parse.h
      {
      }

      /// <summary>
      ///    Creates a new instance with the specified error.
      /// </summary>
      /// <param name="status">The error status</param>
      /// <param name="errorStatement">The statement that failed to be parsed</param>
      /// <param name="errorMsg">The error message given by the native R engine</param>
      public ParseException(ParseStatus status, string errorStatement, string errorMsg)
         : base(MakeErrorMsg(status, errorStatement, errorMsg))
      {
         this.Status = status;
         this.ErrorStatement = errorStatement;
      }

      private static string MakeErrorMsg(ParseStatus status, string errorStatement, string errorMsg)
      {
         return string.Format("Status {2} for {0} : {1}", errorStatement, errorMsg, status);
      }

      /// <summary>
      ///    Creates a new ParseException
      /// </summary>
      /// <param name="info">
      ///    The System.Runtime.Serialization.SerializationInfo that holds the serialised object data about the
      ///    exception being thrown.
      /// </param>
      /// <param name="context"></param>
      protected ParseException(SerializationInfo info, StreamingContext context)
         : base(info, context)
      {
         this.Status = (ParseStatus)info.GetValue(StatusFieldName, typeof(ParseStatus));
         this.ErrorStatement = info.GetString(ErrorStatementFieldName);
      }

      /// <summary>
      ///    The error.
      /// </summary>
      public ParseStatus Status { get; }

      /// <summary>
      ///    The statement caused the error.
      /// </summary>
      public string ErrorStatement { get; }

      /// <summary>
      ///    Sets the serialization info about the exception thrown
      /// </summary>
      /// <param name="info">Serialised object data.</param>
      /// <param name="context">Contextual information about the source or destination</param>
      public override void GetObjectData(SerializationInfo info, StreamingContext context)
      {
         base.GetObjectData(info, context);
         info.AddValue(StatusFieldName, this.Status);
         info.AddValue(ErrorStatementFieldName, this.ErrorStatement);
      }
   }
}