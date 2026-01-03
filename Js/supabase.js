const SUPABASE_URL = "https://kathcwjxdklbdcuewiiw.supabase.co";
const SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImthdGhjd2p4ZGtsYmRjdWV3aWl3Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjczMzY1ODUsImV4cCI6MjA4MjkxMjU4NX0.mcgeqyyh2SHwnyddcieiZ1__whio8H_wG_uiHKnPgGQ";

window.supabase = supabase.createClient(
  SUPABASE_URL,
  SUPABASE_KEY
);
